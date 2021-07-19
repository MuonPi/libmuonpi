#include <muonpi/log.h>

#include <iostream>

auto main() -> int
{
    muonpi::log::system::setup(muonpi::log::Level::Info, [](int c){exit(c);}, std::cerr);

    muonpi::log::warning()<<"This is a warning!";
    muonpi::log::info()<<"This is just some information.";
    muonpi::log::critical(5)<<"Critical!";

    muonpi::log::info()<<"This will not be executed.";

}
