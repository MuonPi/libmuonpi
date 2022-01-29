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
