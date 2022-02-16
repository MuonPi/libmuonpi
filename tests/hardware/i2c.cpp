#include <boost/test/unit_test.hpp>

#include "muonpi/log.h"
#include "muonpi/serial/i2cdevice.h"
#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevices.h"



struct fixture_i2c_handler {
    inline static std::unique_ptr<muonpi::serial::i2c_bus> bus {};

    void setup()
    {
        //muonpi::log::system::setup(muonpi::log::Level::Info, [](int c){exit(c);}, std::cerr);
        bus = std::make_unique<muonpi::serial::i2c_bus>("/dev/i2c-1");
    }

    void teardown()
    {
        //bus->stop();
        //bus->join();
    }
};

BOOST_TEST_GLOBAL_FIXTURE(fixture_i2c_handler);

BOOST_AUTO_TEST_SUITE( test_i2c_access )


BOOST_AUTO_TEST_CASE( test_i2c_access_bus )
{
    auto ok = fixture_i2c_handler::bus->general_call.reset();
    BOOST_TEST( (ok) );
}

BOOST_AUTO_TEST_CASE( test_i2c_access_device )
{
    constexpr std::uint8_t addr { 0xf7 };
    auto& dev = fixture_i2c_handler::bus->open<muonpi::serial::i2c_device>( addr );
    BOOST_TEST( dev.is_open() );
    BOOST_TEST( fixture_i2c_handler::bus->close( addr ) );
}

BOOST_AUTO_TEST_CASE( test_i2c_scan_bus )
{
    for ( std::uint8_t addr = 4; addr < 0x7c; ++addr ) {
        auto& dev = fixture_i2c_handler::bus->open<muonpi::serial::i2c_device>( addr );
        if ( dev.is_open() && dev.present() ) {
            BOOST_TEST( true );
            BOOST_TEST( ( dev.name() != "" ) );
        } else {
            BOOST_TEST( fixture_i2c_handler::bus->close( addr ) );
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
