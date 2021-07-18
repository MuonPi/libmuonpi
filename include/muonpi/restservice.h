#ifndef REST_SERVICE_H
#define REST_SERVICE_H

#include "muonpi/global.h"
#include "muonpi/threadrunner.h"

#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>

#include <queue>
#include <string>
#include <string_view>
#include <vector>

namespace muonpi::rest {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;

using request_type = http::request<http::string_body>;
using response_type = http::response<http::string_body>;
using tcp = net::ip::tcp;

template <http::status Status>
class LIBMUONPI_PUBLIC http_response {
public:
    enum class content_type {
        html,
        json
    };

    http_response(request_type& req, content_type content, std::string application_name = "libmuonpi-" + Version::string())
        : m_response { Status, req.version() }
    {
        m_response.set(http::field::server, application_name);
        std::string content_type_string {};
        switch (content) {
        case content_type::html:
            content_type_string = "text/html";
            break;
        case content_type::json:
            content_type_string = "text/json";
            break;
        }
        m_response.set(http::field::content_type, content_type_string);
        m_response.keep_alive(req.keep_alive());
    }

    http_response(request_type& req)
        : http_response<Status> {req, content_type::html}
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

struct LIBMUONPI_PUBLIC handler {
    std::function<bool(std::string_view path)> matches {};
    std::function<response_type(request_type& req, const std::queue<std::string>& path)> handle {};
    std::string name {};
    bool requires_auth { false };
    std::function<bool(request_type& req, std::string_view username, std::string_view password)> authenticate {};
    std::vector<handler> children {};
};

class LIBMUONPI_PUBLIC service : public thread_runner {
public:

    struct configuration {
        int port {};
        std::string address {};
        bool ssl {};
        std::string cert {};
        std::string privkey {};
        std::string fullchain {};
    };

    service(configuration rest_config);

    void add_handler(handler han);


protected:

    [[nodiscard]] auto custom_run() -> int override;

    void do_accept();

    void on_stop() override;

private:
    [[nodiscard]] auto handle(request_type req) const -> response_type;

    [[nodiscard]] auto handle(request_type req, std::queue<std::string> path, const std::vector<handler>& handlers) const -> response_type;
    [[nodiscard]] auto handle(request_type req, std::queue<std::string> path, const handler& hand) const -> response_type;

    std::vector<handler> m_handler {};

    net::io_context m_ioc { 1 };
    ssl::context m_ctx { ssl::context::tlsv12 };
    tcp::acceptor m_acceptor { m_ioc };
    tcp::endpoint m_endpoint;
    configuration m_rest_conf;
};

}

#endif // REST_SERVICE_H
