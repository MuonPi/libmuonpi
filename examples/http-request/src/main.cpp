#include <muonpi/http_request.h>
#include <muonpi/log.h>

#include <iostream>

auto main() -> int
{
    muonpi::log::system::setup(muonpi::log::Level::Info);

    muonpi::http::destination_t config {};
    config.host = "localhost";
    config.port = 8000;
    config.target = "/hello";
    config.method = muonpi::http::http_verb::get;


    std::cout << muonpi::http::http_request(config, "hello.") << '\n';
}
