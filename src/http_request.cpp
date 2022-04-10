#include "muonpi/http_request.h"

#include "muonpi/scopeguard.h"

#include <future>

#if BOOST_OS_WINDOWS
#include <wincrypt.h>
#endif

namespace muonpi::http {

void load_root_ca(ssl::context& ctx) {
#if BOOST_OS_WINDOWS
    /*
     * +++++++++++++++++++++++++++++++++++++++++++++
     * +++++++ Code block taken unaltered from
     * https://stackoverflow.com/questions/39772878/reliable-way-to-get-root-ca-certificates-on-windows/40710806#40710806
     */

    HCERTSTORE hStore = CertOpenSystemStore(0, "ROOT");
    if (hStore == NULL) {
        return;
    }

    X509_STORE*    store    = X509_STORE_new();
    PCCERT_CONTEXT pContext = NULL;
    while ((pContext = CertEnumCertificatesInStore(hStore, pContext)) != NULL) {
        X509* x509 = d2i_X509(NULL,
                              (const unsigned char**)&pContext->pbCertEncoded,
                              pContext->cbCertEncoded);
        if (x509 != NULL) {
            X509_STORE_add_cert(store, x509);
            X509_free(x509);
        }
    }

    CertFreeCertificateContext(pContext);
    CertCloseStore(hStore, 0);

    SSL_CTX_set_cert_store(ctx.native_handle(), store);

    // ---------------------------------------------
#else
    ctx.set_default_verify_paths();
#endif
}

template <typename Stream>
[[nodiscard]] auto create_request(Stream&                     stream,
                                  const destination_t&        destination,
                                  const std::string&          body,
                                  const std::vector<field_t>& fields) -> response_type {
    // Set up an HTTP GET request message
    request_type req {destination.method, destination.target, destination.version};
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

[[nodiscard]] auto https_request(const destination_t&        destination,
                                 const std::string&          body,
                                 const std::vector<field_t>& fields) -> response_type {
    net::io_context ioc;

    // The SSL context is required, and holds certificates
    ssl::context ctx(ssl::context::tlsv12_client);

    // Verify the remote server's certificate
    ctx.set_verify_mode(ssl::verify_peer);

    load_root_ca(ctx);

    // These objects perform our I/O
    tcp::resolver resolver {ioc};

    detail::ssl_stream_t stream {ioc, ctx};

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream.native_handle(), destination.host.c_str())) {
        beast::error_code ec {static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        fail(ec, "request handshake");
        throw beast::system_error {ec};
    }

    // Look up the domain name
    auto const results = resolver.resolve(destination.host, std::to_string(destination.port));

    // Make the connection on the IP address we get from a lookup

#if BOOST_VERSION < 106900
    boost::asio::connect(stream.next_layer(), results.begin(), results.end());
#else
    beast::get_lowest_layer(stream).connect(results);
#endif

    // Perform the SSL handshake
    stream.handshake(ssl::stream_base::client);

    response_type res {create_request(stream, destination, body, fields)};

    // Gracefully close the stream
    beast::error_code ec;
    stream.shutdown(ec);

    if (ec == net::error::eof) {
        // Rationale:
        // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
        ec = {};
    }

    if (ec) {
        fail(ec, "request shutdown");
        throw beast::system_error {ec};
    }

    return res;
}

auto http_request(const destination_t&        destination,
                  const std::string&          body,
                  const std::vector<field_t>& fields) -> response_type {
    // The io_context is required for all I/O
    net::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver {ioc};

    detail::tcp_stream_t stream {ioc};

    // Look up the domain name
    auto const results = resolver.resolve(destination.host, std::to_string(destination.port));

    // Make the connection on the IP address we get from a lookup
#if BOOST_VERSION < 106900
    boost::asio::connect(stream, results.begin(), results.end());
#else
    stream.connect(results);
#endif

    response_type res {create_request(stream, destination, body, fields)};

    // Write the message to standard out
    // Gracefully close the socket
    beast::error_code ec;
#if BOOST_VERSION < 106900
    stream.shutdown(tcp::socket::shutdown_both, ec);
#else
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);
#endif

    // not_connected happens sometimes
    // so don't bother reporting it.
    if (ec && ec != beast::errc::not_connected) {
        fail(ec, "request shutdown");
        throw beast::system_error {ec};
    }

    return res;
}

auto http_request(const destination_t&        destination,
                  const std::string&          body,
                  bool                        ssl,
                  const std::vector<field_t>& fields) -> response_type {
    if (ssl) {
        return https_request(destination, body, fields);
    }
    return http_request(destination, body, fields);
}

} // namespace muonpi::http
