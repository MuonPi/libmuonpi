#include "muonpi/http_server.h"
#include "muonpi/base64.h"
#include "muonpi/log.h"
#include "muonpi/scopeguard.h"

#include "detail/http_session.hpp"

#include <sstream>
#include <utility>
#include <thread>

namespace muonpi::http {

http_server::http_server(configuration config)
    : thread_runner("http", true)
    , m_endpoint { net::ip::make_address(config.address), static_cast<std::uint16_t>(config.port) }
    , m_conf { std::move(config) }
{
    if (m_conf.ssl) {
        m_ctx.set_options(
            boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2);

        m_ctx.use_private_key_file(m_conf.privkey, ssl::context::file_format::pem);
        m_ctx.use_certificate_file(m_conf.cert, ssl::context::file_format::pem);
        m_ctx.use_certificate_chain_file(m_conf.fullchain);
    }

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

void http_server::add_handler(path_handler han)
{
    m_handler.emplace_back(std::move(han));
}

auto http_server::custom_run() -> int
{
    do_accept();
    m_ioc.run();
    return 0;
}

void http_server::do_accept()
{
    m_acceptor.async_accept([&](const beast::error_code& ec, tcp::socket socket) {
        if (ec) {
            fail(ec, "on accept");
        } else {
            std::thread([&] {
                if (m_conf.ssl) {
                    detail::session<detail::ssl_stream_t> sess { std::move(socket), m_ctx, [&](request_type req) { return handle(std::move(req)); } };
                    sess.run();
                } else {
                    detail::session<detail::tcp_stream_t> sess { std::move(socket), [&](request_type req) { return handle(std::move(req)); } };
                    sess.run();
                }
            }).detach();
            std::this_thread::sleep_for(std::chrono::milliseconds { 2 });
        }
        do_accept();
    });
}

void http_server::on_stop()
{
    m_ioc.stop();
}

auto http_server::handle(request_type req) const -> response_type
{
    if (req.target().empty() || req.target()[0] != '/' || (req.target().find("..") != beast::string_view::npos)) {
        return http_response<beast::http::status::bad_request>(req)("Malformed request-target");
    }
    if (m_handler.empty()) {
        return http_response<beast::http::status::service_unavailable>(req)("No handler installed");
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

auto http_server::handle(request_type req, std::queue<std::string> path, const std::vector<path_handler>& handlers) const -> response_type
{
    while (!path.empty() && path.front().empty()) {
        path.pop();
    }

    if (path.empty()) {
        return http_response<beast::http::status::bad_request>(req)("Request-target empty");
    }

    for (const auto& hand : handlers) {
        if (hand.matches(path.front())) {
            return handle(req, path, hand);
        }
    }
    return http_response<beast::http::status::bad_request>(req)("Illegal request-target");
}

auto http_server::handle(request_type req, std::queue<std::string> path, const path_handler& hand) const -> response_type
{
    path.pop();

    if (hand.requires_auth) {
        std::string auth { req[beast::http::field::authorization] };

        if (auth.empty()) {
            return http_response<beast::http::status::unauthorized>(req)("Need authorisation");
        }
        constexpr std::size_t header_length { 6 };

        auth = base64::decode(auth.substr(header_length));

        auto delimiter = auth.find_first_of(':');
        auto username = auth.substr(0, delimiter);
        auto password = auth.substr(delimiter + 1);

        if (!hand.authenticate(req, username, password)) {
            return http_response<beast::http::status::unauthorized>(req)("Authorisation failed for user: '" + username + "'");
        }
    }

    if (hand.children.empty() || path.empty()) {
        return hand.handle(req, path);
    }

    return handle(std::move(req), std::move(path), hand.children);
}

} // namespace muonpi::http
