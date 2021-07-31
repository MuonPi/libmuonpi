#include <muonpi/log.h>
#include <muonpi/gpio_handler.h>

#include <iostream>

auto main() -> int
{
    muonpi::log::system::setup(muonpi::log::Level::Info, [](int c){exit(c);}, std::cerr);


    muonpi::gpio_handler gpio{"/dev/gpiochip0", "muonpi"};

    auto chip = gpio.get_chip_info();

    std::cout
            <<"chip name: "<<chip.name
            <<"\nchip label: "<<chip.label
            <<"\nchip num lines: "<<chip.num_lines
            <<"\nlines:";
    for (auto name: chip.lines) {
        std::cout<<"\n\t"<<name;
    }
    std::cout<<'\n';



    gpio.join();
}
