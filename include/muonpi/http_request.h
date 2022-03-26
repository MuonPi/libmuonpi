#ifndef MUONPI_HTTP_REQUEST_H
#define MUONPI_HTTP_REQUEST_H

#include "muonpi/global.h"
#include "muonpi/http_tools.h"
#include "muonpi/log.h"

#include <future>

namespace muonpi::http {
/**
 * @brief The destination_t struct
 * Describe the destination of a http request
 */
struct destination_t {
    std::string host {};      //<! The hostname of the destination
    int         port {};      //<! The port to use. Usually 80 or 443 for http and https.
    std::string target {};    //<! The target string to use.
    http_verb   method {};    //<! The http method to use
    int         version {11}; //<! The http version to use. version 1.1 (11) per default.
};

/**
 * @brief The field_t struct
 * Set header fields of the request
 */
struct field_t {
    http_field  field {}; //<! The field to set
    std::string value {}; //<! The value of the field
};

/**
 * @brief http_request Send a http(s) request to a server.
 * @param destination The destination structure
 * @param body The body of the request
 * @param ssl Use https
 * @param fields The header fields to set
 * @return The response from the server
 * @throws beast::system_error
 */
[[nodiscard]] LIBMUONPI_PUBLIC auto http_request(const destination_t&        destination,
                                                 const std::string&          body,
                                                 bool                        ssl    = false,
                                                 const std::vector<field_t>& fields = {})
    -> response_type;

} // namespace muonpi::http

#endif // MUONPI_HTTP_REQUEST_H
