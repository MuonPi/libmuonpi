#include "muonpi/websocket_server.h"
#include "muonpi/scopeguard.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

namespace muonpi::http::ws {

template <typename Stream = beast::tcp_stream>
class session {
public:
    // Take ownership of the socket
    explicit session(tcp::socket&& socket);
    explicit session(tcp::socket&& socket, ssl::context& ctx);

    void set_handler(client_handler handler);

    // Get on the correct executor
    void run();

    // Start the asynchronous operation
    void on_run();

    void on_handshake(beast::error_code ec);

    void on_accept(beast::error_code ec);

    void do_read();

    void on_read(beast::error_code ec, std::size_t bytes_transferred);

    void on_write(beast::error_code ec, std::size_t bytes_transferred);

    void do_write(std::string message);

private:
    void notify();

    websocket::stream<Stream> m_stream;
    beast::flat_buffer m_buffer;
    client_handler m_handler {};

    std::condition_variable m_done {};
    std::mutex m_mutex {};
};

template <>
session<beast::tcp_stream>::session(tcp::socket&& socket)
    : m_stream(std::move(socket))
{
}

template <>
session<beast::ssl_stream<beast::tcp_stream>>::session(tcp::socket&& socket, ssl::context& ctx)
    : m_stream(std::move(socket), ctx)
{
}

template <typename Stream>
void session<Stream>::set_handler(client_handler handler)
{
    m_handler = std::move(handler);
}

template <typename Stream>
void session<Stream>::run()
{
    net::dispatch(m_stream.get_executor(), [&]() { on_run(); });

    std::unique_lock<std::mutex> lock { m_mutex };
    m_done.wait(lock);
    m_handler.on_disconnect();
}

template <>
void session<beast::tcp_stream>::on_run()
{
    scope_guard guard { [&] { notify(); } };
    // Set suggested timeout settings for the websocket
    m_stream.set_option(
        websocket::stream_base::timeout::suggested(
            beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    m_stream.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res) {
            res.set(http_field::server,
                std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-async");
        }));
    // Accept the websocket handshake
    m_stream.async_accept([&](beast::error_code ec) { on_accept(ec); });
    guard.dismiss();
}

template <>
void session<beast::ssl_stream<beast::tcp_stream>>::on_handshake(beast::error_code ec)
{
    scope_guard guard { [&] { notify(); } };
    if (ec) {
        return fail(ec, "handshake");
    }

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(m_stream).expires_never();

    // Set suggested timeout settings for the websocket
    m_stream.set_option(
        websocket::stream_base::timeout::suggested(
            beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    m_stream.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res) {
            res.set(http_field::server,
                std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-async-ssl");
        }));

    m_stream.async_accept([&](beast::error_code ec) { on_accept(ec); });
    guard.dismiss();
}

template <>
void session<beast::ssl_stream<beast::tcp_stream>>::on_run()
{
    scope_guard guard { [&] { notify(); } };
    // Set the timeout.
    beast::get_lowest_layer(m_stream).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    m_stream.next_layer().async_handshake(
        ssl::stream_base::server,
        [&](beast::error_code ec) {
            session<beast::ssl_stream<beast::tcp_stream>>::on_handshake(ec);
        });
    guard.dismiss();
}

template <typename Stream>
void session<Stream>::on_accept(beast::error_code ec)
{
    scope_guard guard { [&] { notify(); } };
    if (ec) {
        return fail(ec, "accept");
    }

    // Read a message
    session<Stream>::do_read();
    guard.dismiss();
}

template <typename Stream>
void session<Stream>::do_read()
{
    scope_guard guard { [&] { notify(); } };
    // Read a message into our buffer
    m_stream.async_read(
        m_buffer,
        [&](beast::error_code ec, std::size_t bytes_transferred) {
            on_read(ec, bytes_transferred);
        });
    guard.dismiss();
}

template <typename Stream>
void session<Stream>::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
    scope_guard guard { [&] { notify(); } };
    boost::ignore_unused(bytes_transferred);

    // This indicates that the session was closed
    if (ec == websocket::error::closed) {
        return;
    }

    if (ec) {
        fail(ec, "read");
    }

    m_handler.on_message(beast::buffers_to_string(m_buffer.data()));

    m_buffer.consume(m_buffer.size());

    do_read();
    guard.dismiss();
}

template <typename Stream>
void session<Stream>::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
    scope_guard guard { [&] { notify(); } };
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        return fail(ec, "write");
    }

    // Clear the buffer
    m_buffer.consume(m_buffer.size());

    // Do another read
    do_read();
    guard.dismiss();
}

template <typename Stream>
void session<Stream>::notify()
{
    m_done.notify_all();
}

template <typename Stream>
void session<Stream>::do_write(std::string message)
{
    // Send the message
    m_stream.async_write(
        net::buffer(message),
        [&](beast::error_code ec, std::size_t bytes_transferred) {
            on_write(ec, bytes_transferred);
        });
}

websocket_server::websocket_server(configuration config, connect_handler handler)
    : thread_runner("websocket", true)
    , m_handler { std::move(handler) }
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

    // Open the acceptor
    m_acceptor.open(m_endpoint.protocol(), ec);
    if (ec) {
        fail(ec, "open");
        return;
    }

    // Allow address reuse
    m_acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    m_acceptor.bind(m_endpoint, ec);
    if (ec) {
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    m_acceptor.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        fail(ec, "listen");
        return;
    }

    start();
}

auto websocket_server::custom_run() -> int
{
    do_accept();
    m_ioc.run();
    return 0;
}

void websocket_server::do_accept()
{
    m_acceptor.async_accept(net::make_strand(m_ioc), [&](const beast::error_code& ec, tcp::socket socket) {
        if (ec) {
            fail(ec, "on accept");
        } else {
            std::thread([&] {
                if (m_conf.ssl) {
                    session<beast::ssl_stream<beast::tcp_stream>> sess { std::move(socket), m_ctx };
                    sess.set_handler(m_handler.on_connect([&](std::string message) { sess.do_write(std::move(message)); }));
                    sess.run();
                } else {
                    session<beast::tcp_stream> sess { std::move(socket) };
                    sess.set_handler(m_handler.on_connect([&](std::string message) { sess.do_write(std::move(message)); }));
                    sess.run();
                }
            }).detach();
            std::this_thread::sleep_for(std::chrono::milliseconds { 2 });
        }
        do_accept();
    });
}

void websocket_server::on_stop()
{
    m_ioc.stop();
}

} // namespace muonpi::http::ws
