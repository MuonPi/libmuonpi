#include "muonpi/http_request.h"

#include "muonpi/scopeguard.h"

#include <future>

namespace muonpi::http {

template <typename Stream>
[[nodiscard]] auto create_request(Stream& stream, destination_t destination, std::string body, std::vector<field_t> fields) -> response_type
{

    // Set up an HTTP GET request message
    request_type req { http_verb::get, destination.target, destination.version };
    req.set(http_field::host, destination.host);
    req.set(http_field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http_field::content_length, std::to_string(body.size()));
    for (const auto& [field, value] : fields) {
        req.set(field, value);
    }
    req.body() = body;

    // Send the HTTP request to the remote host
    beast::http::write(stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    response_type res;

    // Receive the HTTP response
    beast::http::read(stream, buffer, res);

    return res;
}

[[nodiscard]] auto https_request(destination_t destination, std::string body, std::vector<field_t> fields) -> response_type
{
    net::io_context ioc;

    // The SSL context is required, and holds certificates
    ssl::context ctx(ssl::context::tlsv12_client);

    // Verify the remote server's certificate
    ctx.set_verify_mode(ssl::verify_peer);

    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream.native_handle(), destination.host.c_str())) {
        beast::error_code ec { static_cast<int>(::ERR_get_error()), net::error::get_ssl_category() };
        throw beast::system_error { ec };
    }

    // Look up the domain name
    auto const results = resolver.resolve(destination.host, std::to_string(destination.port));

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(stream).connect(results);

    // Perform the SSL handshake
    stream.handshake(ssl::stream_base::client);

    response_type res { create_request(stream, std::move(destination), std::move(body), std::move(fields)) };

    // Gracefully close the stream
    beast::error_code ec;
    stream.shutdown(ec);

    if (ec == net::error::eof) {
        // Rationale:
        // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
        ec = {};
    }

    if (ec) {
        throw beast::system_error { ec };
    }

    return res;
}

auto http_request(destination_t destination, std::string body, std::vector<field_t> fields) -> response_type
{
    // The io_context is required for all I/O
    net::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    // Look up the domain name
    auto const results = resolver.resolve(destination.host, std::to_string(destination.port));

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);

    response_type res { create_request(stream, std::move(destination), std::move(body), std::move(fields)) };

    // Write the message to standard out
    // Gracefully close the socket
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes
    // so don't bother reporting it.
    if (ec && ec != beast::errc::not_connected) {
        throw beast::system_error { ec };
    }

    return res;
}

auto http_request(destination_t destination, std::string body, bool ssl, std::vector<field_t> fields) -> response_type
{
    if (ssl) {
        return https_request(std::move(destination), std::move(body), std::move(fields));
    }
    return http_request(std::move(destination), std::move(body), std::move(fields));
}
}
