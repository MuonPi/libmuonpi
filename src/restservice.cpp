#include "muonpi/restservice.h"
#include "muonpi/base64.h"
#include "muonpi/log.h"
#include "muonpi/scopeguard.h"

#include <sstream>
#include <utility>

namespace muonpi::rest {

void fail(beast::error_code ec, const std::string& what);

class session {
public:
#ifndef PROCESSOR_DISABLE_SSL
    explicit session(tcp::socket&& socket, ssl::context& ctx, std::function<response_type(request_type)> handler);
#else
    explicit session(tcp::socket&& socket, std::function<response_type(request_type)> handler);
#endif

    void run();

    void do_read();

    void do_close();

    void on_read(beast::error_code errorcode, std::size_t bytes_transferred);

    void on_write(bool close, beast::error_code ec, std::size_t bytes_transferred);

private:
    void notify();

#ifndef PROCESSOR_DISABLE_SSL
    beast::ssl_stream<beast::tcp_stream> m_stream;
#else
    beast::tcp_stream m_stream;
#endif

    beast::flat_buffer m_buffer;
    http::request<http::string_body> m_req;
    std::shared_ptr<void> m_res;

    std::function<response_type(request_type)> m_handler;

    std::condition_variable m_done {};
    std::mutex m_mutex {};

    constexpr static std::chrono::duration s_timeout { std::chrono::seconds { 30 } };
};

#ifndef PROCESSOR_DISABLE_SSL
session::session(tcp::socket&& socket, ssl::context& ctx, std::function<response_type(request_type)> handler)
    : m_stream { std::move(socket), ctx }
    , m_handler { std::move(handler) }
{
}
#else
session::session(tcp::socket&& socket, std::function<response_type(request_type)> handler)
    : m_stream { std::move(socket) }
    , m_handler { std::move(handler) }
{
}
#endif

void session::run()
{
    beast::get_lowest_layer(m_stream).expires_after(s_timeout);

#ifndef PROCESSOR_DISABLE_SSL
    m_stream.async_handshake(ssl::stream_base::server, [&](beast::error_code ec) {
        scope_guard guard { [&] { notify(); } };
        if (ec) {
            fail(ec, "handshake");
            return;
        }
        guard.dismiss();
        do_read();
    });
#else
    net::dispatch(m_stream.get_executor(), [&] { do_read(); });
#endif

    std::unique_lock<std::mutex> lock { m_mutex };
    m_done.wait(lock);
}

void session::do_read()
{
    scope_guard guard { [&] { notify(); } };
    m_req = {};

    beast::get_lowest_layer(m_stream).expires_after(s_timeout);

    http::async_read(m_stream, m_buffer, m_req, [&](beast::error_code ec, std::size_t bytes_transferred) { on_read(ec, bytes_transferred); });
    guard.dismiss();
}

void session::do_close()
{
    scope_guard guard { [&] { notify(); } };
    beast::get_lowest_layer(m_stream).expires_after(s_timeout);
#ifndef PROCESSOR_DISABLE_SSL
    m_stream.async_shutdown([&](beast::error_code ec) {
        if (ec) {
            fail(ec, "shutdown");
        }
        notify();
    });
#else
    beast::error_code ec;
    m_stream.socket().shutdown(tcp::socket::shutdown_send, ec);
    if (ec) {
        fail(ec, "shutdown");
    }
#endif
    guard.dismiss();
}

void session::on_read(beast::error_code errorcode, std::size_t bytes_transferred)
{
    scope_guard guard { [&] { notify(); } };
    boost::ignore_unused(bytes_transferred);

    if (errorcode == http::error::end_of_stream) {
        do_close();
        return;
    }

    if (errorcode) {
        fail(errorcode, "read");
        return;
    }

    auto sp { std::make_shared<http::message<false, http::string_body>>(m_handler(std::move(m_req))) };

    m_res = sp;
    http::async_write(
        m_stream,
        *sp,
        [&](beast::error_code ec, std::size_t bytes) { on_write(sp->need_eof(), ec, bytes); });
    guard.dismiss();
}

void session::on_write(bool close, beast::error_code ec, std::size_t bytes_transferred)
{
    scope_guard guard { [&] { notify(); } };
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        fail(ec, "write");
        return;
    }

    if (close) {
        do_close();
        return;
    }

    m_res = nullptr;

    do_read();
    guard.dismiss();
}

void session::notify()
{
    m_done.notify_all();
}

auto service_handler::get_handler() -> handler
{
    return m_handler;
}

void service_handler::set_handler(handler h)
{
    m_handler = std::move(h);
}

service::service(configuration rest_config)
    : thread_runner("REST", true)
    , m_endpoint { net::ip::make_address(rest_config.address), static_cast<std::uint16_t>(rest_config.port) }
    , m_rest_conf { std::move(rest_config) }
{
#ifndef PROCESSOR_DISABLE_SSL
    m_ctx.set_options(
        boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2);

    m_ctx.use_private_key_file(m_rest_conf.privkey, ssl::context::file_format::pem);
    m_ctx.use_certificate_file(m_rest_conf.cert, ssl::context::file_format::pem);
    m_ctx.use_certificate_chain_file(m_rest_conf.fullchain);
#endif // PROCESSOR_DISABLE_SSL

    beast::error_code ec;

    m_acceptor.open(m_endpoint.protocol(), ec);
    if (ec) {
        fail(ec, "open");
        return;
    }

    m_acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
        fail(ec, "set_option");
        return;
    }

    m_acceptor.bind(m_endpoint, ec);
    if (ec) {
        fail(ec, "bind");
        return;
    }

    m_acceptor.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        fail(ec, "listen");
        return;
    }

    start();
}

void service::add_handler(service_handler& han)
{
    m_handler.emplace_back(han.get_handler());
}

auto service::custom_run() -> int
{
    do_accept();
    m_ioc.run();
    return 0;
}

void service::do_accept()
{
    m_acceptor.async_accept([&](const beast::error_code& ec, tcp::socket socket) {
        if (ec) {
            fail(ec, "on accept");
        } else {
            std::thread([&] {
#ifndef PROCESSOR_DISABLE_SSL
                session sess { std::move(socket), m_ctx, [&](request_type req) { return handle(std::move(req)); } };
#else
                session sess { std::move(socket), [&](request_type req) { return handle(std::move(req)); } };
#endif
                sess.run();
            }).detach();
            std::this_thread::sleep_for(std::chrono::milliseconds { 2 });
        }
        do_accept();
    });
}

void service::on_stop()
{
    m_ioc.stop();
}

auto service::handle(request_type req) const -> response_type
{
    if (req.target().empty() || req.target()[0] != '/' || (req.target().find("..") != beast::string_view::npos)) {
        return http_response<http::status::bad_request>(req, "Malformed request-target");
    }
    if (m_handler.empty()) {
        return http_response<http::status::service_unavailable>(req, "No handler installed");
    }

    std::queue<std::string> path {};
    {
        std::istringstream stream { req.target().to_string() };
        for (std::string part; std::getline(stream, part, '/');) {
            path.emplace(part);
        }
    }

    return handle(std::move(req), std::move(path), m_handler);
}

auto service::handle(request_type req, std::queue<std::string> path, const std::vector<handler>& handlers) const -> response_type
{
    while (!path.empty() && path.front().empty()) {
        path.pop();
    }

    if (path.empty()) {
        return http_response<http::status::bad_request>(req, "Request-target empty");
    }

    auto it = handlers.begin();
    for (; it != handlers.end(); it++) {
        if ((*it).matches(path.front())) {
            break;
        }
    }
    if (it == handlers.end()) {
        return http_response<http::status::bad_request>(req, "Illegal request-target");
    }

    const handler& hand { *it };

    path.pop();

    if (hand.requires_auth) {
        std::string auth { req[http::field::authorization] };

        if (auth.empty()) {
            return http_response<http::status::unauthorized>(req, "Need authorisation");
        }
        constexpr std::size_t header_length { 6 };

        auth = base64::decode(auth.substr(header_length));

        auto delimiter = auth.find_first_of(':');
        auto username = auth.substr(0, delimiter);
        auto password = auth.substr(delimiter + 1);

        if (!hand.authenticate(request { req }, username, password)) {
            return http_response<http::status::unauthorized>(req, "Authorisation failed for user: '" + username + "'");
        }
    }

    if (hand.children.empty() || path.empty()) {
        return hand.handle(request { req }, path);
    }

    return handle(std::move(req), std::move(path), hand.children);
}

void fail(beast::error_code ec, const std::string& what)
{
#ifndef PROCESSOR_DISABLE_SSL
    if (ec == net::ssl::error::stream_truncated) {
        return;
    }
#endif

    log::warning() << what << ": " << ec.message();
}

} // namespace muonpi::rest
