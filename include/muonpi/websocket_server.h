#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include "muonpi/global.h"
#include "muonpi/http_tools.h"
#include "muonpi/log.h"
#include "muonpi/threadrunner.h"


#include <queue>
#include <string>
#include <functional>


namespace muonpi::http::ws {

struct LIBMUONPI_PUBLIC client_handler {
    std::function<void(std::string)> on_message;
    std::function<void()> on_disconnect;
};

using connect_callback = std::function<client_handler(std::function<void(std::string)>)>;

struct LIBMUONPI_PUBLIC connect_handler {
    connect_callback on_connect;
};

class LIBMUONPI_PUBLIC websocket_server : public thread_runner {
public:
    struct configuration {
        int port {};
        std::string address {};
        bool ssl {};
        std::string cert {};
        std::string privkey {};
        std::string fullchain {};
    };

    websocket_server(configuration config, connect_handler handler);

protected:
    [[nodiscard]] auto custom_run() -> int override;

    void do_accept();

    void on_stop() override;

private:
    connect_handler m_handler {};

    net::io_context m_ioc;
    ssl::context m_ctx { ssl::context::tlsv12 };
    tcp::acceptor m_acceptor { m_ioc };
    tcp::endpoint m_endpoint;
    configuration m_conf;
};
}

#endif // WEBSOCKET_SERVER_H
