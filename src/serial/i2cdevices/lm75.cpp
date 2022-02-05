#include "muonpi/serial/i2cdevices/lm75.h"
#include "muonpi/scopeguard.h"
#include <cstdint>
#include <iomanip>
#include <iostream>

namespace muonpi::serial::devices {

/*
 * LM75 Temperature Sensor
 */

LM75::LM75(i2c_bus& bus, std::uint8_t address)
    : i2c_device(bus, address)
{
    set_name("LM75");
    m_addresses_hint = { 0x4f };
}

LM75::~LM75() = default;

auto LM75::readRaw() -> std::int16_t
{
    scope_guard timer_guard { setup_timer() };

    std::uint16_t dataword { 0 };
    // Read the temp register
    if (read(static_cast<std::uint8_t>(REG::TEMP), &dataword) != 1) {
        // there was an error
        return INT16_MIN;
    }
    return static_cast<std::int16_t>(dataword);
}

auto LM75::get_temperature() -> float
{
    std::int16_t dataword = readRaw();
    auto temp = static_cast<float>(dataword >> 8);
    temp += static_cast<float>(dataword & 0xff) / 256.;
    return temp;
}

auto LM75::identify() -> bool
{
    if (flag_set(Flags::Failed)) {
        return false;
    }
    if (!present()) {
        return false;
    }
    std::uint8_t conf_reg { 0 };
    // Read the config register
    if (read(static_cast<std::uint8_t>(REG::CONF), &conf_reg) == 0) {
        return false;
    }
    // datasheet: 3 MSBs of conf register "should be kept as zeroes"
    if ((conf_reg >> 5) != 0) {
        return false;
    }

    // read temp register
    std::uint16_t dataword { 0 };
    if (read(static_cast<std::uint8_t>(REG::TEMP), &dataword) == 0) {
        return false;
    }
    // the 5 LSBs should always read zero
    if ((dataword & 0x1f) != 0) {
        return false;
    }

    // read Thyst register
    if (read(static_cast<std::uint8_t>(REG::THYST), &dataword) == 0) {
        return false;
    }
    // the 7 MSBs should always read zero
    if ((dataword & 0x7f) != 0) {
        return false;
    }

    // read Tos register
    if (read(static_cast<std::uint8_t>(REG::TOS), &dataword) == 0) {
        return false;
    }
    // the 7 MSBs should always read zero
    if ((dataword & 0x7f) != 0) {
        return false;
    }

    return true;
}

} // namespace muonpi::serial::devices
