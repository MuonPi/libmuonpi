#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "muonpi/global.h"
#include "muonpi/log.h"
#include "muonpi/http_tools.h"

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


[[nodiscard]] LIBMUONPI_PUBLIC auto http_request(destination_t destination, std::string body, bool ssl = false, std::vector<field_t> fields = {}) -> response_type;


}

#endif // HTTP_REQUEST_H
