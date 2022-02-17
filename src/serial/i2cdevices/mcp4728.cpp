#include "muonpi/serial/i2cdevices/mcp4728.h"

#include "muonpi/scopeguard.h"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>

namespace muonpi::serial::devices {
/*
 * MCP4728 4 ch 12 bit DAC
 */
constexpr auto DataValidityTimeout {std::chrono::milliseconds(100)};

MCP4728::MCP4728(i2c_bus& bus, std::uint8_t address)
    : i2c_device(bus, address) {
    set_name("MCP4728");
    m_addresses_hint = {0x60};
}

auto MCP4728::set_voltage(unsigned int channel, float voltage) -> bool {
    if (voltage < 0 || channel > 3) {
        return false;
    }
    return set_voltage(channel, voltage, false);
}

auto MCP4728::set_voltage(std::uint8_t channel, float voltage, bool toEEPROM) -> bool {
    if (voltage < 0 || channel > 3) {
        return false;
    }
    CFG_GAIN gain = GAIN1;
    // Vref=internal: Vout = (2.048V * Dn) / 4096 * Gx
    // Vref=Vdd: Vout = (Vdd * Dn) / 4096
    std::uint16_t value {0x0000u};
    if (fChannelSetting.at(channel).vref == VREF_2V) {
        value = std::lround(voltage * 2000);
        if (value > 0xfffu) {
            value = value >> 1u;
            gain  = GAIN2;
        }
    } else {
        value = std::lround(voltage * 4096 / fVddRefVoltage);
    }

    if (value > 0xfffu) {
        // error: desired voltage is out of range
        return false;
    }
    return set_value(channel, value, gain, toEEPROM);
}

auto MCP4728::set_value(std::uint8_t channel, std::uint16_t value, CFG_GAIN gain, bool toEEPROM)
    -> bool {
    if (value > 0xfffu) {
        // error: number of bits exceeding 12
        return false;
    }
    DacChannel dacChannel {fChannelSetting.at(channel)};
    if (toEEPROM) {
        dacChannel        = fChannelSettingEep.at(channel);
        dacChannel.eeprom = true;
    }
    dacChannel.value = value;
    dacChannel.gain  = gain;

    return write_channel(channel, dacChannel);
}

auto MCP4728::write_channel(std::uint8_t channel, const DacChannel& channelData) -> bool {
    if (channelData.value > 0xfffu) {
        // error number of bits exceeding 12
        return false;
    }

    scope_guard timer_guard {setup_timer()};

    if (!waitEepReady()) {
        return false;
    }

    channel = channel & 0x03u;
    std::array<std::uint8_t, 3> buf {0, 0, 0};
    if (channelData.eeprom) {
        buf[0] = COMMAND::DAC_EEP_SINGLE_WRITE << 3u;
    } else {
        buf[0] = COMMAND::DAC_MULTI_WRITE << 3u;
    }
    buf[0] |= channel
           << 1u; // 01000/01011 (multiwrite/singlewrite command) DAC1 DAC0 (channel) UDAC bit = 1
    buf[1] = static_cast<std::uint8_t>(channelData.vref)
          << 7u; // Vref PD1 PD0 Gx (gain) D11 D10 D9 D8
    buf[1] |= (channelData.pd & 0x03u) << 5u;
    buf[1] |= static_cast<std::uint8_t>((channelData.value & 0xf00u) >> 8u);
    buf[1] |= static_cast<std::uint8_t>(channelData.gain & 0x01u) << 4u;
    buf[2] = static_cast<std::uint8_t>(channelData.value & 0xffu); // D7 D6 D5 D4 D3 D2 D1 D0
    if (write(buf.data(), 3u) != 3) {
        // somehow did not write exact same amount of bytes as it should
        return false;
    }

    if (channelData.eeprom) {
        fChannelSettingEep.at(channel) = channelData;
    } else {
        fChannelSetting.at(channel) = channelData;
    }
    // force a register update next time anything is read
    fLastRegisterUpdate = {};
    return true;
}

auto MCP4728::store_settings() -> bool {
    scope_guard timer_guard {setup_timer()};

    const std::uint8_t startchannel {0};

    if (!waitEepReady()) {
        return false;
    }

    std::array<std::uint8_t, 9> buf {};
    buf[0] = COMMAND::DAC_EEP_SEQ_WRITE << 3u;
    buf[0] |= startchannel << 1u; // command DAC1 DAC0 UDAC
    for (std::size_t channel = 0; channel < 4u; channel++) {
        buf.at(channel * 2u + 1u) = static_cast<std::uint8_t>(fChannelSetting.at(channel).vref)
                                 << 7u; // Vref PD1 PD0 Gx (gain) D11 D10 D9 D8
        buf.at(channel * 2u + 1u) |= (fChannelSetting.at(channel).pd & 0x03u) << 5u;
        buf.at(channel * 2u + 1u) |=
            static_cast<std::uint8_t>((fChannelSetting.at(channel).value & 0xf00u) >> 8u);
        buf.at(channel * 2u + 1u) |=
            static_cast<std::uint8_t>(fChannelSetting.at(channel).gain & 0x01u) << 4u;
        buf.at(channel * 2u + 2u) = static_cast<std::uint8_t>(fChannelSetting.at(channel).value
                                                              & 0xffu); // D7 D6 D5 D4 D3 D2 D1 D0
    }

    if (write(buf.data(), 9u) != 9) {
        // somehow did not write exact same amount of bytes as it should
        return false;
    }
    // force a register update next time anything is read
    fLastRegisterUpdate = {};

    return true;
}

auto MCP4728::present() -> bool {
    return read_registers();
}

auto MCP4728::waitEepReady() -> bool {
    if (!read_registers()) {
        return false;
    }
    int timeout_ctr {100};
    while (fBusy && timeout_ctr-- > 0) {
        fLastRegisterUpdate = {};
        if (!read_registers()) {
            return false;
        }
    }
    return timeout_ctr > 0;
}

auto MCP4728::read_registers() -> bool {
    scope_guard timer_guard {setup_timer()};

    // perform a register update only if the buffered content is too old
    if ((std::chrono::steady_clock::now() - fLastRegisterUpdate) < DataValidityTimeout) {
        return true;
    }
    // perform a read sequence of all registers as described in datasheet
    std::array<std::uint8_t, 24> buf {};
    if (24 != read(buf.data(), 24u)) {
        return false;
    }
    parse_channel_data(buf.data());
    fLastRegisterUpdate = std::chrono::steady_clock::now();
    return true;
}

auto MCP4728::read_channel(std::uint8_t channel, bool eeprom) -> std::optional<DacChannel> {
    if (channel > 3u) {
        // error: channel index exceeding 3
        return std::nullopt;
    }

    if (!read_registers()) {
        return std::nullopt;
    }
    if (eeprom) {
        return std::optional<DacChannel> {fChannelSettingEep.at(channel)};
    }
    return std::optional<DacChannel> {fChannelSetting.at(channel)};
}

auto MCP4728::code2voltage(const DacChannel& channelData) -> float {
    float vref    = (channelData.vref == VREF_2V) ? 2.048 : fVddRefVoltage;
    float voltage = vref * channelData.value / 4096.;
    if (channelData.gain == GAIN2 && channelData.vref != VREF_VDD) {
        voltage *= 2.;
    }
    return voltage;
}

auto MCP4728::identify() -> bool {
    if (flag_set(Flags::Failed)) {
        return false;
    }
    if (!present()) {
        return false;
    }
    // perform a read sequence of all 24 bytes// explicitely do not use the read_registers() method
    // to prevent parsing of the raw data values into meaningful content
    // TODO: Use named constants.
    std::array<std::uint8_t, 24> buf {};
    if (24 != read(buf.data(), 24u)) {
        return false;
    }

    return ((buf[0u] & 0xf0u) == 0xc0u) && ((buf[6u] & 0xf0u) == 0xd0u)
        && ((buf[12u] & 0xf0u) == 0xe0u) && ((buf[18u] & 0xf0u) == 0xf0u);
}

auto MCP4728::set_vref(unsigned int channel, CFG_VREF vref_setting) -> bool {
    if (!waitEepReady()) {
        return false;
    }

    channel = channel & 0x03u;
    std::uint8_t databyte {0};
    databyte = COMMAND::VREF_WRITE >> 1u;
    for (std::size_t ch = 0; ch < 4u; ch++) {
        if (ch == channel) {
            databyte |= static_cast<std::uint8_t>(vref_setting);
        } else {
            databyte |= static_cast<std::uint8_t>(fChannelSetting.at(ch).vref);
        }
        databyte = databyte << 1u;
    }

    scope_guard timer_guard {setup_timer()};

    if (write(&databyte, 1u) != 1) {
        // somehow did not write exact same amount of bytes as it should
        return false;
    }

    fChannelSetting.at(channel).vref = vref_setting;
    return true;
}

auto MCP4728::set_vref(CFG_VREF vref_setting) -> bool {
    if (!waitEepReady()) {
        return false;
    }
    std::uint8_t databyte {0};
    databyte = COMMAND::VREF_WRITE >> 1u;
    for (std::size_t ch = 0; ch < 4u; ch++) {
        databyte |= static_cast<std::uint8_t>(vref_setting);
        databyte = databyte << 1u;
    }

    scope_guard timer_guard {setup_timer()};

    if (write(&databyte, 1u) != 1) {
        // somehow did not write exact same amount of bytes as it should
        return false;
    }

    fChannelSetting.at(0).vref = fChannelSetting.at(1).vref = fChannelSetting.at(2).vref =
        fChannelSetting.at(3).vref                          = vref_setting;
    return true;
}

void MCP4728::parse_channel_data(const std::uint8_t* buf) {
    for (std::size_t channel = 0; channel < 4u; channel++) {
        // dac reg: offs = 1
        // eep: offs = 4
        fChannelSetting.at(channel).vref =
            (buf[channel * 6u + 1u] & 0x80u) != 0 ? VREF_2V : VREF_VDD;
        fChannelSettingEep.at(channel).vref =
            (buf[channel * 6u + 4u] & 0x80u) != 0 ? VREF_2V : VREF_VDD;

        fChannelSetting.at(channel).pd    = (buf[channel * 6u + 1u] & 0x60u) >> 5u;
        fChannelSettingEep.at(channel).pd = (buf[channel * 6u + 4u] & 0x60u) >> 5u;

        fChannelSetting.at(channel).gain    = (buf[channel * 6u + 1u] & 0x10u) != 0 ? GAIN2 : GAIN1;
        fChannelSettingEep.at(channel).gain = (buf[channel * 6u + 4u] & 0x10u) != 0 ? GAIN2 : GAIN1;

        fChannelSetting.at(channel).value =
            static_cast<std::uint16_t>(buf[channel * 6u + 1u] & 0x0fu) << 8u;
        fChannelSetting.at(channel).value |=
            static_cast<std::uint16_t>(buf[channel * 6u + 2u] & 0xffu);

        fChannelSettingEep.at(channel).value =
            static_cast<std::uint16_t>(buf[channel * 6u + 4u] & 0x0fu) << 8u;
        fChannelSettingEep.at(channel).value |=
            static_cast<std::uint16_t>(buf[channel * 6u + 5u] & 0xffu);

        fBusy = (buf[21u] & 0x80u) == 0;

        fChannelSettingEep.at(channel).eeprom = true;
    }
}

void MCP4728::dump_registers() {
    std::array<std::uint8_t, 24> buf {};
    if (24 != read(buf.data(), 24u)) {
        return;
    }
    for (std::size_t ch = 0; ch < 4u; ch++) {
        std::cout << "DAC" << ch << ": " << std::setw(2) << std::setfill('0') << std::hex
                  << (int)buf[ch * 6u] << " " << (int)buf[ch * 6u + 1u] << " "
                  << (int)buf[ch * 6u + 2u] << " (eep: " << (int)buf[ch * 6u + 3u] << " "
                  << (int)buf[ch * 6u + 4u] << " " << (int)buf[ch * 6u + 5u] << ")\n";
    }
}

} // namespace muonpi::serial::devices
