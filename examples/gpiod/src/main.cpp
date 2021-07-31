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

    if (gpio.set_pin_interrupt(5, muonpi::gpio::edge_t::Both, muonpi::gpio::bias_t::Disabled, [](muonpi::gpio::event_t evt){
        std::cout
            <<evt.pin
            <<": "<<((evt.edge==muonpi::gpio::edge_t::Rising)?"Rising":"Falling")
            <<": "<<std::chrono::duration_cast<std::chrono::microseconds>(evt.time.time_since_epoch()).count()<<"\n";
	
	}))
    {
         std::cout<<"success.\n";
    } else {
         std::cout<<"fail\n";
    }

    gpio.join();
}
