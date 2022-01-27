#ifndef MUONPI_SERIAL_I2CDEVICES_MCP4728_H
#define MUONPI_SERIAL_I2CDEVICES_MCP4728_H

#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"

#include <chrono>

namespace muonpi::serial::devices {

/* MCP4728 4ch 12bit DAC */
class MCP4728 : public i2c_device {
    // the DAC supports writing to input register but not sending latch bit to update the output register
    // here we will always send the "UDAC" (latch) bit because we don't need this functionality
    // MCP4728 listens to I2C Generall Call Commands
    // reset, wake-up, software update, read address bits
    // reset is "0x00 0x06"
    // wake-up is "0x00 0x09"
public:
    enum CFG_GAIN { GAIN1 = 0,
        GAIN2 = 1 };
    enum CFG_VREF { VREF_VDD = 0,
        VREF_2V = 1 };

    // struct that characterizes one dac output channel
    // setting the eeprom flag enables access to the eeprom registers instead of the dac output registers
    struct DacChannel {
        uint8_t pd = 0x00;
        CFG_GAIN gain = GAIN1;
        CFG_VREF vref = VREF_2V;
        bool eeprom = false;
        uint16_t value = 0;
    };

    MCP4728(i2c_bus& bus, std::uint8_t address);

    [[nodiscard]] auto present() -> bool override;
    [[nodiscard]] auto set_voltage(unsigned int channel, float voltage) -> bool;
    [[nodiscard]] auto store_settings() -> bool;
    [[nodiscard]] auto write_channel(uint8_t channel, const DacChannel& channelData) -> bool;
    [[nodiscard]] auto read_channel(uint8_t channel, DacChannel& channelData) -> bool; // TODO: Do not use output parameters. Use std::optional<DacChannel> instead.
    [[nodiscard]] auto set_vref(unsigned int channel, CFG_VREF vref_setting) -> bool;
    [[nodiscard]] auto set_vref(CFG_VREF vref_setting) -> bool;

    static auto code2voltage(const DacChannel& channelData) -> float;

    [[nodiscard]] auto identify() -> bool override;

private:
    static constexpr float fVddRefVoltage { 3.3 }; ///< voltage at which the device is powered
    enum COMMAND : uint8_t {
        DAC_FAST_WRITE = 0b00000000,
        DAC_MULTI_WRITE = 0b00001000,
        DAC_EEP_SEQ_WRITE = 0b00001010,
        DAC_EEP_SINGLE_WRITE = 0b00001011,
        ADDR_BITS_WRITE = 0b00001100,
        VREF_WRITE = 0b00010000,
        GAIN_WRITE = 0b00011000,
        PD_WRITE = 0b00010100
    };
    DacChannel fChannelSetting[4], fChannelSettingEep[4]; // TODO: Don't use C-style arrays. Use std::array instead.
    // TODO: Don't declare multiple members in one statement
    std::chrono::time_point<std::chrono::steady_clock> fLastRegisterUpdate {};
    bool fBusy { false };

    auto set_voltage(uint8_t channel, float voltage, bool toEEPROM) -> bool;
    auto set_value(uint8_t channel, uint16_t value, CFG_GAIN gain = GAIN1, bool toEEPROM = false) -> bool;
    auto read_registers() -> bool;
    void parse_channel_data(const uint8_t* buf);
    void dump_registers();
    auto waitEepReady() -> bool;
};

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_MCP4728_H
