#include <muonpi/configuration.h>
#include <muonpi/log.h>

#include <iostream>

auto main(int argc, const char* argv[]) -> int
{
    muonpi::log::system::setup(muonpi::log::Level::Info, [](int c){exit(c);}, std::cerr);

    auto cmd_options = muonpi::config::setup("Commandline options");
    cmd_options.add_option("test,t", "This is just a test!");
    cmd_options.add_option("var,v", boost::program_options::value<std::string>(), "This is also just a test!");
    cmd_options.add_option("int,i", boost::program_options::value<int>()->required(), "This is also just a test!");
    cmd_options.commit(argc, argv);

    std::cout<<cmd_options;


    auto file_options = muonpi::config::setup("Commandline options");
    file_options.add_option("ftest", boost::program_options::value<std::string>(), "This is just a test!");
    file_options.add_option("fvar", boost::program_options::value<std::string>(), "This is also just a test!");
    file_options.add_option("fint", boost::program_options::value<int>()->required(), "This is also just a test!");
    file_options.commit("example.cfg");

    std::cout<<file_options;

    if (muonpi::config::is_set("var")) {
        std::cout<<"option var: "<<muonpi::config::get<std::string>("var")<<'\n';
    }
    std::cout<<"option int: "<<muonpi::config::get<int>("int")<<'\n';
}
