#ifndef HTTP_TOOLS_H
#define HTTP_TOOLS_H

#include "muonpi/log.h"

#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
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

}
#endif // HTTP_TOOLS_H
