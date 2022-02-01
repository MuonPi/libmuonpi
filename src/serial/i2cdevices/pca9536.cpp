#include "muonpi/serial/i2cdevices/pca9536.h"
#include "muonpi/scopeguard.h"
#include <cstdint>
#include <iomanip>
#include <iostream>

namespace muonpi::serial::devices {

/*
 * PCA9536 4 pin I/O expander
 */

PCA9536::PCA9536(i2c_bus& bus, std::uint8_t address)
    : i2c_device(bus, address)
{
    set_name("PCA9536");
}

auto PCA9536::set_direction_mask(std::uint8_t output_mask) -> bool
{
    std::uint8_t data = ~output_mask;
    scope_guard timer_guard { setup_timer() };
    if (1 != write(REG::CONFIG, &data, 1)) {
        return false;
    }
    return true;
}

auto PCA9536::set_output_states(std::uint8_t state_mask) -> bool
{
    scope_guard timer_guard { setup_timer() };
    if (1 != write(REG::OUTPUT, &state_mask, 1)) {
        return false;
    }
    return true;
}

auto PCA9536::get_input_states() -> std::optional<std::uint8_t>
{
    std::uint8_t inport { 0x00 };
    scope_guard timer_guard { setup_timer() };
    if (1 != read(REG::INPUT, &inport, 1)) {
        return std::nullopt;
    }
    return std::optional<std::uint8_t> { inport & 0x0f };
}

auto PCA9536::present() -> bool
{
    std::uint8_t inport { 0x00 };
    // read input port pattern
    return (1 == read(REG::INPUT, &inport, 1));
}

auto PCA9536::identify() -> bool
{
    if (flag_set(Flags::Failed)) {
        return false;
    }
    if (!present()) {
        return false;
    }

    std::uint8_t bytereg { 0 };
    if (read(static_cast<std::uint8_t>(REG::INPUT), &bytereg) == 0) {
        // there was an error
        return false;
    }
    if ((bytereg & 0xf0) != 0xf0) {
        return false;
    }
    if (read(static_cast<std::uint8_t>(REG::POLARITY), &bytereg) == 0) {
        return false;
    }
    if ((bytereg & 0xf0) != 0x00) {
        return false;
    }

    if (read(static_cast<std::uint8_t>(REG::CONFIG), &bytereg) == 0) {
        return false;
    }
    if ((bytereg & 0xf0) != 0xf0) {
        return false;
    }
    if (read(static_cast<std::uint8_t>(0x04), &bytereg, 1) > 0) {
        return false;
    }
    return true;
}

} // namespace muonpi::serial::devices
