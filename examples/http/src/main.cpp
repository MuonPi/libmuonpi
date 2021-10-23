#include <muonpi/http_server.h>
#include <muonpi/log.h>

#include <iostream>

auto main() -> int
{
    muonpi::log::system::setup(muonpi::log::Level::Info);

    muonpi::http::http_server::configuration config {};
    config.address = "0.0.0.0";
    config.port = 8000;
    config.ssl = false;

    muonpi::http::http_server service{config};

    muonpi::http::path_handler handler1 {};
    handler1.matches = [](std::string_view path) {return path == "hello";};
    handler1.handle = [](muonpi::http::request_type& req, const std::queue<std::string>& /*unused*/) -> muonpi::http::response_type {
        std::cout<<"Got request for /hello\n"<<std::flush;
        return muonpi::http::http_response<muonpi::http::http_status::ok>(req)("Hello!");
    };

    service.add_handler(handler1);

    muonpi::http::path_handler handler2 {};
    handler2.matches = [](std::string_view path) {return path == "bye";};
    handler2.handle = [](muonpi::http::request_type& req, const std::queue<std::string>& /*unused*/) -> muonpi::http::response_type {
        std::cout<<"Got request for /bye\n"<<std::flush;
        return muonpi::http::http_response<muonpi::http::http_status::ok>(req)("Bye!");
    };

    muonpi::http::path_handler handler3 {};
    handler3.matches = [](std::string_view path) {return path == "bye";};
    handler3.handle = [](muonpi::http::request_type& req, const std::queue<std::string>& /*unused*/) -> muonpi::http::response_type {
        std::cout<<"Got request for /bye/bye\n"<<std::flush;
        return muonpi::http::http_response<muonpi::http::http_status::ok>(req)("Bye-Bye!");
    };

    handler2.children.emplace_back(handler3);

    service.add_handler(handler2);

    service.join();
}
