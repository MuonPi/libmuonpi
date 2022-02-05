#include "muonpi/serial/i2cdevices/mic184.h"
#include "muonpi/scopeguard.h"
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <thread>

namespace muonpi::serial::devices {

/*
 * MIC184 Temperature Sensor
 */
MIC184::MIC184(i2c_bus& bus, std::uint8_t address)
    : i2c_device(bus, address)
{
    set_name("MIC184");
    m_addresses_hint = { 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f };
}

MIC184::~MIC184() = default;

auto MIC184::readRaw() -> std::int16_t
{
    scope_guard timer_guard { setup_timer() };

    std::uint16_t dataword { 0 };
    // Read the temp register
    if (read(static_cast<std::uint8_t>(REG::TEMP), &dataword) == 0) {
        // there was an error
        return INT16_MIN;
    }

    return static_cast<std::int16_t>(dataword);
}

auto MIC184::get_temperature() -> float
{
    std::int16_t dataword = readRaw();
    auto temp = static_cast<float>(dataword >> 8);
    temp += static_cast<float>(dataword & 0xff) / 256.;
    return temp;
}

auto MIC184::identify() -> bool
{
    if (flag_set(Flags::Failed)) {
        return false;
    }
    if (!present()) {
        return false;
    }

    // Read and backup the config register
    std::uint8_t conf_reg_save { 0 };
    if (read(static_cast<std::uint8_t>(REG::CONF), &conf_reg_save) == 0) {
        return false;
    }
    // datasheet: the interrupt mask bit in conf register should be zero when device is in init state
    if ((conf_reg_save & 0b01000000) != 0) {
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

    // read and backup Thyst register
    std::uint16_t thyst_save { 0 };
    if (read(static_cast<std::uint8_t>(REG::THYST), &thyst_save) == 0) {
        return false;
    }
    // the 7 MSBs should always read zero
    if ((thyst_save & 0x7f) != 0) {
        return false;
    }

    // read and backup Tos register
    std::uint16_t tos_save { 0 };
    if (read(static_cast<std::uint8_t>(REG::TOS), &tos_save) == 0) {
        return false;
    }
    // the 7 MSBs should always read zero
    if ((tos_save & 0x7f) != 0) {
        return false;
    }

    // determine, whether we have a MIC184 or just a plain LM75
    // datasheet: test, if the STS (status) bit in config register toggles when a alarm condition is provoked
    // set config reg to 0x02
    std::uint8_t conf_reg { 0x02 };
    if (write(static_cast<std::uint8_t>(REG::CONF), &conf_reg) == 0) {
        return false;
    }
    // write 0xc880 to Thyst and Tos regs. This corresponds to -55.5 degrees centigrade
    dataword = 0xc880;
    if (write(static_cast<std::uint8_t>(REG::THYST), &dataword) == 0) {
        return false;
    }
    if (write(static_cast<std::uint8_t>(REG::TOS), &dataword) == 0) {
        return false;
    }
    // wait at least one conversion cycle (>160ms)
    std::this_thread::sleep_for(std::chrono::milliseconds(160));
    // Read back config register
    if (read(static_cast<std::uint8_t>(REG::CONF), &conf_reg) == 0) {
        return false;
    }
    // datasheet: MSB of conf reg should be set to one
    // this is considered an indication for MIC184
    if ((conf_reg & 0x80) == 0) {
        return false;
    }

    // write 0x7f80 to Thyst and Tos regs. This corresponds to +127.5 degrees centigrade
    dataword = 0x7f80;
    if (write(static_cast<std::uint8_t>(REG::THYST), &dataword) == 0) {
        return false;
    }
    if (write(static_cast<std::uint8_t>(REG::TOS), &dataword) == 0) {
        return false;
    }
    // wait at least one conversion cycle (>160ms)
    std::this_thread::sleep_for(std::chrono::milliseconds(160));
    // Read the config register again to clear pending interrupt request
    if (read(static_cast<std::uint8_t>(REG::CONF), &conf_reg) == 0) {
        return false;
    }

    // at this point we know for sure that the device is an MIC184
    // set THyst and Tos regs back to previous settings
    write(static_cast<std::uint8_t>(REG::THYST), &thyst_save);
    write(static_cast<std::uint8_t>(REG::TOS), &tos_save);
    // finally, set config reg into original state
    if (write(static_cast<std::uint8_t>(REG::CONF), &conf_reg_save) != 0) {
        m_external = ((conf_reg_save & 0x20) != 0);
        return true;
    }
    return false;
}

auto MIC184::set_external(bool enable_external) -> bool
{
    // Read and save the config register
    std::uint8_t conf_reg { 0 };
    std::uint8_t conf_reg_save { 0 };
    if (read(static_cast<std::uint8_t>(REG::CONF), &conf_reg) == 0) {
        return false;
    }
    conf_reg_save = conf_reg;
    // disable interrupts, clear IM bit
    conf_reg &= ~0x40;
    if (write(static_cast<std::uint8_t>(REG::CONF), &conf_reg) == 0) {
        return false;
    }
    // read back config reg to clear STS flag
    if (read(static_cast<std::uint8_t>(REG::CONF), &conf_reg) == 0) {
        return false;
    }
    if (enable_external) {
        conf_reg_save |= 0x20;
    } else {
        conf_reg_save &= ~0x20;
    }
    if (write(static_cast<std::uint8_t>(REG::CONF), &conf_reg_save) == 0) {
        return false;
    }
    if (read(static_cast<std::uint8_t>(REG::CONF), &conf_reg) == 0) {
        return false;
    }
    if ((conf_reg & 0x20) != (conf_reg_save & 0x20)) {
        return false;
    }
    m_external = enable_external;
    // wait one cycle until a conversion in the new zone is ready
    std::this_thread::sleep_for(std::chrono::milliseconds(160));
    // and wait twice as long if external zone enabled (datasheet: tconv=160ms (int) and 320ms (ext))
    if (m_external) {
        std::this_thread::sleep_for(std::chrono::milliseconds(160));
    }
    return true;
}

auto MIC184::is_external() const -> bool
{
    return m_external;
}

} // namespace muonpi::serial::devices
