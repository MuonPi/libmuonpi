#ifndef MUONPI_HTTP_RESPONSE_H
#define MUONPI_HTTP_RESPONSE_H

#include "muonpi/global.h"
#include "muonpi/http_tools.h"
#include "muonpi/log.h"
#include "muonpi/threadrunner.h"

#include <queue>
#include <string>
#include <string_view>
#include <vector>

namespace muonpi::http {

/**
 * @brief The content_type struct
 */
struct content_type {
    /**
     * @brief html Get the content type string text/html
     */
    [[nodiscard]] static auto html() noexcept -> content_type;

    /**
     * @brief json Get the content type string text/json
     */
    [[nodiscard]] static auto json() noexcept -> content_type;

    std::string string {};
};

template <beast::http::status Status>
/**
 * @brief The http_response class
 * Construct http responses from the server
 */
class LIBMUONPI_PUBLIC http_response {
public:
    /**
     * @brief http_response
     * @param req The type of request
     * @param content The content type string
     * @param application_name The name of the application in the http header
     */
    http_response(request_type&       req,
                  const content_type& content,
                  const std::string&  application_name = "libmuonpi-"
                                                      + Version::libmuonpi::string());

    /**
     * @brief http_response
     * Construct a default response
     * @param req The request type
     */
    explicit http_response(request_type& req);

    /**
     * @brief commit Complete the response
     * @param body The body to attach to the response
     */
    [[nodiscard]] auto commit(std::string body) -> response_type;

    /**
     * @brief operator () Complete the response
     * @param body The body to attach to the response
     */
    [[nodiscard]] auto operator()(std::string body) -> response_type;

private:
    response_type m_response;
};

template <beast::http::status Status>
http_response<Status>::http_response(request_type&       req,
                                     const content_type& content,
                                     const std::string&  application_name)
    : m_response {Status, req.version()} {
    m_response.set(beast::http::field::server, application_name);
    m_response.set(beast::http::field::content_type, content.string);
    m_response.keep_alive(req.keep_alive());
}

template <beast::http::status Status>
http_response<Status>::http_response(request_type& req)
    : http_response<Status> {req, content_type::html()} {}

template <beast::http::status Status>
auto http_response<Status>::commit(std::string body) -> response_type {
    m_response.body() = std::move(body);
    m_response.prepare_payload();
    return std::move(m_response);
}

template <beast::http::status Status>
auto http_response<Status>::operator()(std::string body) -> response_type {
    return std::move(commit(std::move(body)));
}

} // namespace muonpi::http

#endif // MUONPI_HTTP_RESPONSE_H
