#include <boost/test/unit_test.hpp>

#include "muonpi/gpio_handler.h"



struct fixture_gpio_handler {
    inline static muonpi::gpio_handler gpio {"/dev/gpiochip0", "libmuonpi-test"};
};

BOOST_TEST_GLOBAL_FIXTURE(fixture_gpio_handler);

BOOST_AUTO_TEST_SUITE( test_gpio_access )


BOOST_AUTO_TEST_CASE( test_gpio_access_init )
{
	auto chip = fixture_gpio_handler::gpio.get_chip_info();
	BOOST_TEST( (!chip.name.empty()) );
	BOOST_TEST( (chip.num_lines > 0) );
}

BOOST_AUTO_TEST_SUITE_END()
