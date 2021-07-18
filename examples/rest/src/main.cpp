#include <muonpi/restservice.h>

#include <iostream>

auto main() -> int
{
    muonpi::rest::service::configuration config {};
    config.address = "127.0.0.1";
    config.port = 8000;
    config.ssl = false;

    muonpi::rest::service service{config};

    muonpi::rest::handler handler1 {};
    handler1.matches = [](std::string_view path) {return path == "hello";};
    handler1.handle = [](muonpi::rest::request_type& req, const std::queue<std::string>&) -> muonpi::rest::response_type {
        std::cout<<"Got request for /hello\n"<<std::flush;
        return muonpi::rest::http_response<muonpi::rest::http::status::ok>(req)("Hello!");
    };

    service.add_handler(handler1);

    muonpi::rest::handler handler2 {};
    handler2.matches = [](std::string_view path) {return path == "bye";};
    handler2.handle = [](muonpi::rest::request_type& req, const std::queue<std::string>&) -> muonpi::rest::response_type {
        std::cout<<"Got request for /bye\n"<<std::flush;
        return muonpi::rest::http_response<muonpi::rest::http::status::ok>(req)("Bye!");
    };

    muonpi::rest::handler handler3 {};
    handler3.matches = [](std::string_view path) {return path == "bye";};
    handler3.handle = [](muonpi::rest::request_type& req, const std::queue<std::string>&) -> muonpi::rest::response_type {
        std::cout<<"Got request for /bye/bye\n"<<std::flush;
        return muonpi::rest::http_response<muonpi::rest::http::status::ok>(req)("Bye-Bye!");
    };

    handler2.children.emplace_back(handler3);

    service.add_handler(handler2);

    service.join();
}
