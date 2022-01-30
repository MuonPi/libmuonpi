#include <boost/test/unit_test.hpp>

#include "muonpi/gpio_handler.h"


BOOST_AUTO_TEST_SUITE( detector_gpio_tests )

BOOST_AUTO_TEST_CASE( test_gpio_state )
{
	muonpi::gpio::state_t state {};

	BOOST_TEST( (state == muonpi::gpio::state_t::Undefined) );

	state = !state;

	BOOST_TEST( (state == muonpi::gpio::state_t::Undefined) );

	state = { muonpi::gpio::state_t::Low };

	BOOST_TEST( (state == muonpi::gpio::state_t::Low) );

	state = !state;

	BOOST_TEST( (state == muonpi::gpio::state_t::High) );

	state = 3;

	BOOST_TEST( (state == muonpi::gpio::state_t { muonpi::gpio::state_t::High }) );

	BOOST_TEST( (state == muonpi::gpio::state_t::High) );

	int int_state { state };

	BOOST_TEST( (int_state == muonpi::gpio::state_t::High) );
	BOOST_TEST( (int_state == 1) );

	int_state = static_cast<int>(~state);
	
	BOOST_TEST( (int_state == muonpi::gpio::state_t::Low) );
	BOOST_TEST( (int_state == 0) );

	bool bool_state { state };

	BOOST_TEST( (bool_state == true) );
	BOOST_TEST( (bool_state == muonpi::gpio::state_t::High) );
	
	bool_state = !bool_state;

	BOOST_TEST( (bool_state == false) );
	BOOST_TEST( (bool_state == muonpi::gpio::state_t::Low) );
}
BOOST_AUTO_TEST_SUITE_END()
