#include "muonpi/http_response.h"

#include "muonpi/http_tools.h"

namespace muonpi::http {

auto content_type::html() noexcept -> content_type
{
    return content_type { "text/html" };
}

auto content_type::json() noexcept -> content_type
{
    return content_type { "text/json" };
}

} // namespace muonpi::http