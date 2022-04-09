#include <cstdint>
#include <iomanip>
#include <iostream>
#include <muonpi/addressrange.h>
#include <muonpi/log.h>
#include <muonpi/serial/i2cbus.h>
#include <muonpi/serial/i2cdevices/generalcall.h>
#include <sstream>

using namespace muonpi;

/**
 * @brief The any_device class
 * Example for a device class which opens the first device it can find.
 */
class any_device : public serial::i2c_device {
public:
    // construct an address_range object with base address 0x00 and mask 0x7c, so all bits are
    // variable.
    constexpr static address_range addresses {0x00, {0x7c}};

    // This constructor should not be used.
    any_device(traffic_t&, const std::string&, i2c_device::address_type) = delete;

    any_device(traffic_t& bus_traffic, const std::string& path)
        : i2c_device {bus_traffic, path, addresses} {}
};

auto main() -> int {
    log::system::setup(
        log::Level::Info,
        [](int c) { exit(c); },
        std::cerr);

    serial::i2c_bus bus {"/dev/i2c-1"};
    if (!bus.get<serial::general_call>().reset()) {
        log::error() << "resetting bus through general call command";
    }

    log::info() << "scanning bus " << bus.path() << " for devices...";

    auto& dev = bus.get<any_device>();
    if (dev.is_open() && dev.present()) {
        log::info() << "found " << dev.name() << " at 0x" << std::hex << std::setw(2)
                    << std::setfill('0') << static_cast<int>(dev.address());
    } else {
        bus.close(dev.address());
    }
}
