#ifndef MUONPI_HTTP_SERVER_H
#define MUONPI_HTTP_SERVER_H

#include "muonpi/global.h"
#include "muonpi/http_tools.h"
#include "muonpi/log.h"
#include "muonpi/threadrunner.h"

#include <queue>
#include <string>
#include <string_view>
#include <vector>

namespace muonpi::http {

template <beast::http::status Status>
class LIBMUONPI_PUBLIC http_response {
public:
    enum class content_type {
        html,
        json
    };

    http_response(request_type& req, content_type content, const std::string& application_name = "libmuonpi-" + Version::libmuonpi::string())
        : m_response { Status, req.version() }
    {
        m_response.set(beast::http::field::server, application_name);
        std::string content_type_string {};
        switch (content) {
        case content_type::html:
            content_type_string = "text/html";
            break;
        case content_type::json:
            content_type_string = "text/json";
            break;
        }
        m_response.set(beast::http::field::content_type, content_type_string);
        m_response.keep_alive(req.keep_alive());
    }

    explicit http_response(request_type& req)
        : http_response<Status> { req, content_type::html }
    {
    }

    [[nodiscard]] auto commit(std::string body) -> response_type
    {
        m_response.body() = std::move(body);
        m_response.prepare_payload();
        return std::move(m_response);
    }

    [[nodiscard]] auto operator()(std::string body) -> response_type
    {
        return std::move(commit(std::move(body)));
    }

private:
    response_type m_response;
};

struct LIBMUONPI_PUBLIC path_handler {
    std::function<bool(std::string_view path)> matches {};
    std::function<response_type(request_type& req, const std::queue<std::string>& path)> handle {};
    std::string name {};
    bool requires_auth { false };
    std::function<bool(request_type& req, std::string_view username, std::string_view password)> authenticate {};
    std::vector<path_handler> children {};
};

class LIBMUONPI_PUBLIC http_server : public thread_runner {
public:
    struct configuration {
        int port {};
        std::string address {};
        bool ssl {};
        std::string cert {};
        std::string privkey {};
        std::string fullchain {};
    };

    explicit http_server(configuration config);

    void add_handler(path_handler han);

protected:
    [[nodiscard]] auto custom_run() -> int override;

    void do_accept();

    void on_stop() override;

private:
    [[nodiscard]] auto handle(request_type req) const -> response_type;

    [[nodiscard]] auto handle(request_type req, std::queue<std::string> path, const std::vector<path_handler>& handlers) const -> response_type;
    [[nodiscard]] auto handle(request_type req, std::queue<std::string> path, const path_handler& hand) const -> response_type;

    std::vector<path_handler> m_handler {};

    net::io_context m_ioc { 1 };
    ssl::context m_ctx { ssl::context::tlsv12 };
    tcp::acceptor m_acceptor { m_ioc };
    tcp::endpoint m_endpoint;
    configuration m_conf;
};

} // namespace muonpi::http

#endif // MUONPI_HTTP_SERVER_H
