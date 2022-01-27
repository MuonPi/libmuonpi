#include "muonpi/serial/i2cdevices/mcp4728.h"
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>

namespace muonpi::serial::devices {
/*
 * MCP4728 4 ch 12 bit DAC
 */
constexpr auto DataValidityTimeout { std::chrono::milliseconds(100) };

MCP4728::MCP4728(i2c_bus& bus, std::uint8_t address)
    : i2c_device(bus, address)
{
    set_title("MCP4728");
}

auto MCP4728::set_voltage(unsigned int channel, float voltage) -> bool
{
    if (voltage < 0 || channel > 3) {
        return false;
    }
    return set_voltage(channel, voltage, false);
}

auto MCP4728::set_voltage(uint8_t channel, float voltage, bool toEEPROM) -> bool
{
    if (voltage < 0 || channel > 3) {
        return false;
    }
    CFG_GAIN gain = GAIN1;
    // Vref=internal: Vout = (2.048V * Dn) / 4096 * Gx
    // Vref=Vdd: Vout = (Vdd * Dn) / 4096
    uint16_t value { 0x0000 };
    if (fChannelSetting[channel].vref == VREF_2V) {
        value = std::lround(voltage * 2000);
        if (value > 0xfff) {
            value = value >> 1;
            gain = GAIN2;
        }
    } else {
        value = std::lround(voltage * 4096 / fVddRefVoltage);
    }

    if (value > 0xfff) {
        // error: desired voltage is out of range
        return false;
    }
    return set_value(channel, value, gain, toEEPROM);
}

auto MCP4728::set_value(uint8_t channel, uint16_t value, CFG_GAIN gain, bool toEEPROM) -> bool
{
    if (value > 0xfff) {
        value = 0xfff;
        // error: number of bits exceeding 12
        return false;
    }
    DacChannel dacChannel { fChannelSetting[channel] };
    if (toEEPROM) {
        dacChannel = fChannelSettingEep[channel];
        dacChannel.eeprom = true;
    }
    dacChannel.value = value;
    dacChannel.gain = gain;

    return write_channel(channel, dacChannel);
}

auto MCP4728::write_channel(uint8_t channel, const DacChannel& channelData) -> bool
{
    if (channelData.value > 0xfff) {
        // error number of bits exceeding 12
        return false;
    }
    start_timer();
    if (!waitEepReady()) {
        return false;
    }

    channel = channel & 0x03;
    uint8_t buf[3]; // TODO: Do not use c-style arrays. Use std::array instead.
    if (channelData.eeprom) {
        buf[0] = COMMAND::DAC_EEP_SINGLE_WRITE << 3;
    } else {
        buf[0] = COMMAND::DAC_MULTI_WRITE << 3;
    }
    buf[0] |= (channel << 1); // 01000/01011 (multiwrite/singlewrite command) DAC1 DAC0 (channel) UDAC bit = 1
    buf[1] = ((uint8_t)channelData.vref) << 7; // Vref PD1 PD0 Gx (gain) D11 D10 D9 D8
    buf[1] |= (channelData.pd & 0x03) << 5;
    buf[1] |= (uint8_t)((channelData.value & 0xf00) >> 8);
    buf[1] |= (uint8_t)(channelData.gain & 0x01) << 4;
    buf[2] = (uint8_t)(channelData.value & 0xff); // D7 D6 D5 D4 D3 D2 D1 D0
    if (write(buf, 3) != 3) {
        // somehow did not write exact same amount of bytes as it should
        return false;
    }
    stop_timer();

    if (channelData.eeprom) {
        fChannelSettingEep[channel] = channelData;
    } else {
        fChannelSetting[channel] = channelData;
    }
    // force a register update next time anything is read
    fLastRegisterUpdate = {};
    return true;
}

auto MCP4728::store_settings() -> bool
{
    start_timer();
    const uint8_t startchannel { 0 };
    uint8_t buf[9]; // TODO: Do not use c-style arrays. Use std::array instead.

    if (!waitEepReady()) {
        return false;
    }

    buf[0] = COMMAND::DAC_EEP_SEQ_WRITE << 3;
    buf[0] |= (startchannel << 1); // command DAC1 DAC0 UDAC
    for (uint8_t channel = 0; channel < 4; channel++) {
        buf[channel * 2 + 1] = ((uint8_t)fChannelSetting[channel].vref) << 7; // Vref PD1 PD0 Gx (gain) D11 D10 D9 D8
        buf[channel * 2 + 1] |= (fChannelSetting[channel].pd & 0x03) << 5;
        buf[channel * 2 + 1] |= (uint8_t)((fChannelSetting[channel].value & 0xf00) >> 8);
        buf[channel * 2 + 1] |= (uint8_t)(fChannelSetting[channel].gain & 0x01) << 4;
        buf[channel * 2 + 2] = (uint8_t)(fChannelSetting[channel].value & 0xff); // D7 D6 D5 D4 D3 D2 D1 D0
    }

    if (write(buf, 9) != 9) {
        // somehow did not write exact same amount of bytes as it should
        return false;
    }
    stop_timer();
    // force a register update next time anything is read
    fLastRegisterUpdate = {};

    return true;
}

auto MCP4728::present() -> bool
{
    return read_registers();
}

auto MCP4728::waitEepReady() -> bool
{
    if (!read_registers()) {
        return false;
    }
    int timeout_ctr { 100 };
    while (fBusy && timeout_ctr-- > 0) {
        fLastRegisterUpdate = {};
        if (!read_registers()) {
            return false;
        }
    }
    return timeout_ctr > 0;
}

auto MCP4728::read_registers() -> bool
{
    uint8_t buf[24];
    start_timer();
    // perform a register update only if the buffered content is too old
    if ((std::chrono::steady_clock::now() - fLastRegisterUpdate) < DataValidityTimeout) {
        return true;
    }
    // perform a read sequence of all registers as described in datasheet
    if (24 != read(buf, 24)) {
        return false;
    }
    stop_timer();
    parse_channel_data(buf);
    fLastRegisterUpdate = std::chrono::steady_clock::now();
    return true;
}

auto MCP4728::read_channel(uint8_t channel, DacChannel& channelData) -> bool
{
    if (channel > 3) {
        // error: channel index exceeding 3
        return false;
    }

    if (!read_registers()) {
        return false;
    }
    if (channelData.eeprom) {
        channelData = fChannelSettingEep[channel];
    } else {
        channelData = fChannelSetting[channel];
    }

    return true;
}

auto MCP4728::code2voltage(const DacChannel& channelData) -> float
{
    float vref = (channelData.vref == VREF_2V) ? 2.048 : fVddRefVoltage;
    float voltage = vref * channelData.value / 4096.;
    if (channelData.gain == GAIN2 && channelData.vref != VREF_VDD) {
        voltage *= 2.;
    }
    return voltage;
}

auto MCP4728::identify() -> bool
{
    if (flag_set(Flags::Failed)) {
        return false;
    }
    if (!present()) {
        return false;
    }
    // TODO: Use named constants.
    // TODO: Do not use c-style arrays. Use std::array instead.
    uint8_t buf[24];
    if (read(buf, 24) != 24) {
        return false;
    }

    return ((buf[0] & 0xf0) == 0xc0) && ((buf[6] & 0xf0) == 0xd0) && ((buf[12] & 0xf0) == 0xe0) && ((buf[18] & 0xf0) == 0xf0);
}

auto MCP4728::set_vref(unsigned int channel, CFG_VREF vref_setting) -> bool
{
    start_timer();
    if (!waitEepReady()) {
        return false;
    }

    channel = channel & 0x03;
    uint8_t databyte { 0 };
    databyte = COMMAND::VREF_WRITE >> 1;
    for (unsigned int ch = 0; ch < 4; ch++) { // TODO: Use std::size_t instead of unsigned int
        if (ch == channel) {
            databyte |= vref_setting;
        } else {
            databyte |= fChannelSetting[ch].vref;
        }
        databyte = (databyte << 1);
    }

    if (write(&databyte, 1) != 1) {
        // somehow did not write exact same amount of bytes as it should
        return false;
    }
    stop_timer();

    fChannelSetting[channel].vref = vref_setting;
    return true;
}

auto MCP4728::set_vref(CFG_VREF vref_setting) -> bool
{
    start_timer();
    if (!waitEepReady()) {
        return false;
    }
    uint8_t databyte { 0 };
    databyte = COMMAND::VREF_WRITE >> 1;
    for (unsigned int ch = 0; ch < 4; ch++) {
        databyte |= vref_setting;
        databyte = (databyte << 1);
    }

    if (write(&databyte, 1) != 1) {
        // somehow did not write exact same amount of bytes as it should
        return false;
    }
    stop_timer();

    fChannelSetting[0].vref = fChannelSetting[1].vref = fChannelSetting[2].vref = fChannelSetting[3].vref = vref_setting;
    return true;
}

void MCP4728::parse_channel_data(const uint8_t* buf)
{
    for (unsigned int channel = 0; channel < 4; channel++) {
        // dac reg: offs = 1
        // eep: offs = 4
        fChannelSetting[channel].vref = (buf[channel * 6 + 1] & 0x80) != 0 ? VREF_2V : VREF_VDD;
        fChannelSettingEep[channel].vref = (buf[channel * 6 + 4] & 0x80) != 0 ? VREF_2V : VREF_VDD;

        fChannelSetting[channel].pd = (buf[channel * 6 + 1] & 0x60) >> 5;
        fChannelSettingEep[channel].pd = (buf[channel * 6 + 4] & 0x60) >> 5;

        fChannelSetting[channel].gain = (buf[channel * 6 + 1] & 0x10) != 0 ? GAIN2 : GAIN1;
        fChannelSettingEep[channel].gain = (buf[channel * 6 + 4] & 0x10) != 0 ? GAIN2 : GAIN1;

        fChannelSetting[channel].value = (uint16_t)(buf[channel * 6 + 1] & 0x0f) << 8; // TODO: Don't use c-style casts. Use static_cast instead.
        fChannelSetting[channel].value |= (uint16_t)(buf[channel * 6 + 1 + 1] & 0xff); // TODO: Don't use c-style casts. Use static_cast instead.

        fChannelSettingEep[channel].value = (uint16_t)(buf[channel * 6 + 4] & 0x0f) << 8; // TODO: Don't use c-style casts. Use static_cast instead.
        fChannelSettingEep[channel].value |= (uint16_t)(buf[channel * 6 + 4 + 1] & 0xff); // TODO: Don't use c-style casts. Use static_cast instead.

        fBusy = (buf[21] & 0x80) == 0;

        fChannelSettingEep[channel].eeprom = true;
    }
}

void MCP4728::dump_registers()
{
    uint8_t buf[24]; // TODO: Don't use c-style arrays. Use std::array instead.
    if (read(buf, 24) != 24) {
        // somehow did not read exact same amount of bytes as it should
        return;
    }
    for (int ch = 0; ch < 4; ch++) {
        std::cout << "DAC" << ch << ": " << std::setw(2) << std::setfill('0') << std::hex
                  << (int)buf[ch * 6] << " " << (int)buf[ch * 6 + 1] << " " << (int)buf[ch * 6 + 2]
                  << " (eep: " << (int)buf[ch * 6 + 3] << " " << (int)buf[ch * 6 + 4] << " " << (int)buf[ch * 6 + 5] << ")\n";
    }
}

} // namespace muonpi::serial::devices
