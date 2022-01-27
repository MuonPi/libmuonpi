#ifndef MUONPI_HTTP_SESSION_H
#define MUONPI_HTTP_SESSION_H
#include "muonpi/base64.h"
#include "muonpi/http_tools.h"
#include "muonpi/log.h"
#include "muonpi/scopeguard.h"

#include <condition_variable>
#include <sstream>
#include <utility>

namespace muonpi::http::detail {

template <typename Stream>
class session {
public:
    explicit session(tcp::socket&& socket, ssl::context& ctx, std::function<response_type(request_type)> handler);
    explicit session(tcp::socket&& socket, std::function<response_type(request_type)> handler);

    void run();

    void do_read();

    void do_close();

    void on_read(beast::error_code errorcode, std::size_t bytes_transferred);

    void on_write(bool close, beast::error_code ec, std::size_t bytes_transferred);

private:
    void notify();

    Stream m_stream;

    beast::flat_buffer m_buffer;
    request_type m_req;
    std::shared_ptr<void> m_res;

    std::function<response_type(request_type)> m_handler;

    std::condition_variable m_done {};
    std::mutex m_mutex {};

    constexpr static std::chrono::duration s_timeout { std::chrono::seconds { 30 } };
};

template <>
session<ssl_stream_t>::session(tcp::socket&& socket, ssl::context& ctx, std::function<response_type(request_type)> handler)
    : m_stream { std::move(socket), ctx }
    , m_handler { std::move(handler) }
{
}

template <>
session<tcp_stream_t>::session(tcp::socket&& socket, std::function<response_type(request_type)> handler)
    : m_stream { std::move(socket) }
    , m_handler { std::move(handler) }
{
}

template <>
void session<ssl_stream_t>::run()
{
#if BOOST_VERSION >= 106900
    beast::get_lowest_layer(m_stream).expires_after(s_timeout);
#endif
    m_stream.async_handshake(ssl::stream_base::server, [&](beast::error_code ec) {
        scope_guard guard { [&] { notify(); } };
        if (ec) {
            fail(ec, "handshake");
            return;
        }
        guard.dismiss();
        do_read();
    });

    std::unique_lock<std::mutex> lock { m_mutex };
    m_done.wait(lock);
}

template <>
void session<tcp_stream_t>::run()
{
#if BOOST_VERSION >= 106900
    beast::get_lowest_layer(m_stream).expires_after(s_timeout);
#endif
    net::dispatch(m_stream.get_executor(), [&] { do_read(); });

    std::unique_lock<std::mutex> lock { m_mutex };
    m_done.wait(lock);
}

template <>
void session<ssl_stream_t>::do_close()
{
#if BOOST_VERSION >= 106900
    beast::get_lowest_layer(m_stream).expires_after(s_timeout);
#endif
    scope_guard guard { [&] { notify(); } };
    m_stream.async_shutdown([&](beast::error_code ec) {
        if (ec) {
            fail(ec, "shutdown");
        }
        notify();
    });
    guard.dismiss();
}

template <>
void session<tcp_stream_t>::do_close()
{
    scope_guard guard { [&] { notify(); } };
    beast::error_code ec;
#if BOOST_VERSION >= 106900
    beast::get_lowest_layer(m_stream).expires_after(s_timeout);
    m_stream.socket().shutdown(tcp::socket::shutdown_send, ec);
#else
    m_stream.shutdown(tcp::socket::shutdown_send, ec);
#endif
    if (ec) {
        fail(ec, "shutdown");
    }
    guard.dismiss();
}

template <typename Stream>
void session<Stream>::do_read()
{
    scope_guard guard { [&] { notify(); } };
    m_req = {};

#if BOOST_VERSION >= 106900
    beast::get_lowest_layer(m_stream).expires_after(s_timeout);
#endif
    beast::http::async_read(m_stream, m_buffer, m_req, [&](beast::error_code ec, std::size_t bytes_transferred) { on_read(ec, bytes_transferred); });
    guard.dismiss();
}

template <typename Stream>
void session<Stream>::on_read(beast::error_code errorcode, std::size_t bytes_transferred)
{
    scope_guard guard { [&] { notify(); } };
    boost::ignore_unused(bytes_transferred);

    if (errorcode == beast::http::error::end_of_stream) {
        do_close();
        return;
    }

    if (errorcode) {
        fail(errorcode, "read");
        return;
    }

    auto sp { std::make_shared<beast::http::message<false, beast::http::string_body>>(m_handler(std::move(m_req))) };

    bool close { sp->need_eof() };
    m_res = sp;
    beast::http::async_write(
        m_stream,
        *sp,
        [&](beast::error_code ec, std::size_t bytes) {
            on_write(close, ec, bytes);
        });
    guard.dismiss();
}

template <typename Stream>
void session<Stream>::on_write(bool close, beast::error_code ec, std::size_t bytes_transferred)
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

template <typename Stream>
void session<Stream>::notify()
{
    m_done.notify_all();
}

} // namespace muonpi::http::detail

#endif //  MUONPI_HTTP_SESSION_H
