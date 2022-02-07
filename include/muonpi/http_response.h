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
    [[nodiscard]] inline static auto html() noexcept -> content_type;

    /**
     * @brief json Get the content type string text/json
     */
    [[nodiscard]] inline static auto json() noexcept -> content_type;

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
    http_response(request_type& req, const content_type& content, const std::string& application_name = "libmuonpi-" + Version::libmuonpi::string());

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

} // namespace muonpi::http

#endif // MUONPI_HTTP_RESPONSE_H
