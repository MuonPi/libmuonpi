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

/**
 * @brief The path_handler struct
 */
struct LIBMUONPI_PUBLIC path_handler {
    std::function<bool(std::string_view path)>
        matches {}; //<! Function to determine whether the current path of the request matches this
        // handler
    std::function<response_type(request_type& req, const std::queue<std::string>& path)>
        handle {}; //<! The registered handler function which gets called by the server
    std::string name {}; //<! The name of this handler
    bool requires_auth { false }; //<! Whether this handler requires authentication
    std::function<bool(request_type& req, std::string_view username, std::string_view password)>
        authenticate {}; //<! The authentication handler to use
    std::vector<path_handler> children {}; //<! Child handlers of this handler.
};

/**
 * @brief The http_server class
 */
class LIBMUONPI_PUBLIC http_server : public thread_runner {
public:
    /**
     * @brief The configuration struct
     */
    struct configuration {
        int port {}; //<! The port on which to listen
        std::string address {}; //<! The bind address to use

        bool ssl {}; //<! whether to use ssl or not.
        std::string cert {}; //<! The certificate file to use
        std::string privkey {}; //<! The private key to use
        std::string fullchain {}; //<! The full keychain to use
    };

    /**
     * @brief http_server
     * @param config The configuration to use
     */
    explicit http_server(configuration config);

    /**
     * @brief add_handler Add a path handler to the server
     * @param handler The handler to add
     */
    void add_handler(path_handler handler);

protected:
    [[nodiscard]] auto custom_run() -> int override;

    void do_accept();

    void on_stop() override;

private:
    [[nodiscard]] auto handle(request_type req) const -> response_type;

    [[nodiscard]] auto handle(request_type req,
        std::queue<std::string> path,
        const std::vector<path_handler>& handlers) const -> response_type;
    [[nodiscard]] auto handle(request_type req,
        std::queue<std::string> path,
        const path_handler& hand) const -> response_type;

    std::vector<path_handler> m_handler {};

    net::io_context m_ioc { 1 };
    ssl::context m_ctx { ssl::context::tlsv12 };
    tcp::acceptor m_acceptor { m_ioc };
    tcp::endpoint m_endpoint;
    configuration m_conf;
};

} // namespace muonpi::http

#endif // MUONPI_HTTP_SERVER_H
