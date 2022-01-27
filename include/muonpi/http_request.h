#ifndef MUONPI_HTTP_REQUEST_H
#define MUONPI_HTTP_REQUEST_H

#include "muonpi/global.h"
#include "muonpi/http_tools.h"
#include "muonpi/log.h"

#include <future>

namespace muonpi::http {
struct destination_t {
    std::string host {};
    int port {};
    std::string target {};
    http_verb method {};
    int version { 11 };
};

struct field_t {
    http_field field {};
    std::string value {};
};

[[nodiscard]] LIBMUONPI_PUBLIC auto http_request(const destination_t& destination, const std::string& body, bool ssl = false, const std::vector<field_t>& fields = {}) -> response_type;

} // namespace muonpi::http

#endif // MUONPI_HTTP_REQUEST_H
