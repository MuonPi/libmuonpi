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
            <<"\nlines:\n";
    unsigned int gpio_index { 0 };
    for (const auto& line: chip.lines) {
        std::cout << gpio_index++ << "\t: " << line.name << " " << line.consumer << "\n";
    }
    std::cout<<'\n';

    auto led_set_fn { gpio.set_pin_output( 19, muonpi::gpio::state_t{muonpi::gpio::state_t::Low}, muonpi::gpio::bias_t::OpenSource ) };
    auto gpio_read_fn { gpio.get_pin_input( 23, muonpi::gpio::bias_t::Disabled ) };

    muonpi::gpio::pins_t pins {
        muonpi::gpio::settings_t{5, muonpi::gpio::edge_t::Falling, muonpi::gpio::bias_t::Disabled},
        muonpi::gpio::settings_t{27, muonpi::gpio::edge_t::Rising, muonpi::gpio::bias_t::Disabled},
    };

    auto callback { [&](muonpi::gpio::event_t evt){
        std::cout
            <<evt.pin
            <<": "<<((evt.edge==muonpi::gpio::edge_t::Rising)?"Rising":"Falling")
            <<": "<<std::chrono::duration_cast<std::chrono::microseconds>(evt.time.time_since_epoch()).count()<<"\n";
        // read state of the pin with the provided lambda function
        muonpi::gpio::state_t state = gpio_read_fn();
        // set state of other pin with this state
        bool ok = led_set_fn(!state);
        if (!ok) {
            std::cout<<" error setting LED\n";
        }
    } };

    if (gpio.set_pin_interrupt(pins, callback))
    {
         std::cout<<"success.\n";
    } else {
         std::cout<<"fail\n";
    }

    gpio.join();
}
