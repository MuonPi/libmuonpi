#include <boost/test/unit_test.hpp>

#include "muonpi/gpio_handler.h"



struct fixture_gpio_handler {
    inline static std::unique_ptr<muonpi::gpio_handler> gpio {};

    void setup()
    {
        muonpi::log::system::setup(muonpi::log::Level::Info, [](int c){exit(c);}, std::cerr);
        gpio = std::make_unique<muonpi::gpio_handler>("/dev/gpiochip0", "libmuonpi-test");
    }

    void teardown()
    {
        gpio->stop();
        gpio->join();
    }
};

BOOST_TEST_GLOBAL_FIXTURE(fixture_gpio_handler);

BOOST_AUTO_TEST_SUITE( test_gpio_access )


BOOST_AUTO_TEST_CASE( test_gpio_access_init )
{
    auto chip = fixture_gpio_handler::gpio->get_chip_info();
    BOOST_TEST( (!chip.name.empty()) );
    BOOST_TEST( (chip.num_lines > 0) );
}

BOOST_AUTO_TEST_CASE( test_gpio_access_read )
{
    auto gpio_read_fn { fixture_gpio_handler::gpio->get_pin_input( 23, muonpi::gpio::bias_t::Disabled ) };
    muonpi::gpio::state_t state { gpio_read_fn() };
    BOOST_TEST( (state != muonpi::gpio::state_t::Undefined) );
}

BOOST_AUTO_TEST_SUITE_END()
