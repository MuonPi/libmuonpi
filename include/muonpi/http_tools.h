#ifndef HTTP_TOOLS_H
#define HTTP_TOOLS_H

#include "muonpi/log.h"

#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/version.hpp>

#if BOOST_VERSION < 106900
#include <boost/asio/connect.hpp>
#include <boost/asio/ssl/stream.hpp>
#else
#include <boost/beast/ssl.hpp>
#endif

#include <boost/asio/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/config.hpp>

namespace muonpi::http {

namespace beast = boost::beast;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;

using request_type = beast::http::request<beast::http::string_body>;
using response_type = beast::http::response<beast::http::string_body>;
using tcp = net::ip::tcp;
namespace websocket = beast::websocket;

using http_verb = beast::http::verb;
using http_field = beast::http::field;
using http_status = beast::http::status;

void fail(beast::error_code ec, const std::string& what);

namespace detail {
#if BOOST_VERSION < 106900
    using ssl_stream_t = ssl::stream<tcp::socket>;
    using tcp_stream_t = tcp::socket;
#else
    using ssl_stream_t = beast::ssl_stream<beast::tcp_stream>;
    using tcp_stream_t = beast::tcp_stream;
#endif
}

}
#endif // HTTP_TOOLS_H
