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

template <beast::http::status Status>
http_response<Status>::http_response(request_type& req, const content_type& content, const std::string& application_name)
    : m_response { Status, req.version() }
{
    m_response.set(beast::http::field::server, application_name);
    m_response.set(beast::http::field::content_type, content.string);
    m_response.keep_alive(req.keep_alive());
}

template <beast::http::status Status>
http_response<Status>::http_response(request_type& req)
    : http_response<Status> { req, content_type::html() }
{
}

template <beast::http::status Status>
auto http_response<Status>::commit(std::string body) -> response_type
{
    m_response.body() = std::move(body);
    m_response.prepare_payload();
    return std::move(m_response);
}

template <beast::http::status Status>
auto http_response<Status>::operator()(std::string body) -> response_type
{
    return std::move(commit(std::move(body)));
}

} // namespace muonpi::http