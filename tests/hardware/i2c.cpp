#include "muonpi/log.h"
#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"
#include "muonpi/serial/i2cdevices.h"

#include <boost/test/unit_test.hpp>

struct fixture_i2c_handler {
    inline static std::unique_ptr<muonpi::serial::i2c_bus> bus {};

    void setup() {
        // muonpi::log::system::setup(muonpi::log::Level::Info, [](int c){exit(c);}, std::cerr);
        bus = std::make_unique<muonpi::serial::i2c_bus>("/dev/i2c-1");
    }

    void teardown() {}
};

BOOST_TEST_GLOBAL_FIXTURE(fixture_i2c_handler);

BOOST_AUTO_TEST_SUITE(test_i2c_access)

BOOST_AUTO_TEST_CASE(test_i2c_general_call) {
    auto ok = fixture_i2c_handler::bus->general_call.reset();
    BOOST_TEST((ok));
}

BOOST_AUTO_TEST_CASE(test_i2c_access_bus) {
    constexpr std::uint8_t addr {0xf7};
    auto&                  dev = fixture_i2c_handler::bus->open<muonpi::serial::i2c_device>(addr);
    BOOST_TEST(dev.is_open());
    BOOST_TEST(fixture_i2c_handler::bus->close(addr));
}

BOOST_AUTO_TEST_CASE(test_i2c_mic184) {
    auto found_tempsensors =
        fixture_i2c_handler::bus->identify_devices<muonpi::serial::devices::MIC184>(
            muonpi::serial::devices::MIC184::default_addresses());
    for (auto addr : found_tempsensors) {
        // found the specific device at the expected position, so close the previously created
        // generic i2c_device and reopen as temp sensor device
        // fixture_i2c_handler::bus->close(addr);
        auto& tempsensor = fixture_i2c_handler::bus->open<muonpi::serial::devices::MIC184>(addr);
        auto  temp       = tempsensor.get_temperature();
        BOOST_TEST((temp >= -55. && temp <= 127.));
        BOOST_TEST((tempsensor.last_access_duration().count() > 0
                    && tempsensor.last_access_duration() < std::chrono::seconds(1)));
    }
}

BOOST_AUTO_TEST_CASE(test_i2c_ads1115) {
    auto found_adcs = fixture_i2c_handler::bus->identify_devices<muonpi::serial::devices::ADS1115>(
        muonpi::serial::devices::ADS1115::default_addresses());
    for (auto addr : found_adcs) {
        // found the specific device at the expected position, so close the previously created
        // generic i2c_device and reopen as temp sensor device
        // fixture_i2c_handler::bus->close(addr);
        auto& adc     = fixture_i2c_handler::bus->open<muonpi::serial::devices::ADS1115>(addr);
        auto  voltage = adc.getVoltage(0);
        BOOST_TEST((voltage >= -5. && voltage <= 5.));
        BOOST_TEST((adc.last_access_duration().count() > 0
                    && adc.last_access_duration() < std::chrono::seconds(1)));
    }
}

BOOST_AUTO_TEST_CASE(test_i2c_mcp4728) {
    auto found_dacs = fixture_i2c_handler::bus->identify_devices<muonpi::serial::devices::MCP4728>(
        muonpi::serial::devices::MCP4728::default_addresses());
    for (auto addr : found_dacs) {
        // found the specific device at the expected position, so close the previously created
        // generic i2c_device and reopen as adc device
        // fixture_i2c_handler::bus->close(addr);
        auto& dac         = fixture_i2c_handler::bus->open<muonpi::serial::devices::MCP4728>(addr);
        auto  dac_channel = dac.read_channel(0);
        BOOST_TEST(dac_channel.has_value());
        BOOST_TEST((dac.last_access_duration().count() > 0
                    && dac.last_access_duration() < std::chrono::seconds(1)));
    }
}

BOOST_AUTO_TEST_SUITE_END()
