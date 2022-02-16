#ifndef MUONPI_SERIAL_I2CDEVICES_MCP4728_H
#define MUONPI_SERIAL_I2CDEVICES_MCP4728_H

#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"

#include <chrono>
#include <optional>

namespace muonpi::serial::devices {

/* MCP4728 4ch 12bit DAC */
class MCP4728 : public i2c_device {
    // the DAC supports writing to input register but not sending latch bit to update the output
    // register here we will always send the "UDAC" (latch) bit because we don't need this
    // functionality MCP4728 listens to I2C Generall Call Commands reset, wake-up, software update,
    // read address bits reset is "0x00 0x06" wake-up is "0x00 0x09"
public:
    enum CFG_GAIN
    {
        GAIN1 = 0,
        GAIN2 = 1
    };
    enum CFG_VREF
    {
        VREF_VDD = 0,
        VREF_2V  = 1
    };

    // struct that characterizes one dac output channel
    // setting the eeprom flag enables access to the eeprom registers instead of the dac output
    // registers
    struct DacChannel {
        std::uint8_t  pd     = 0x00;
        CFG_GAIN      gain   = GAIN1;
        CFG_VREF      vref   = VREF_2V;
        bool          eeprom = false;
        std::uint16_t value  = 0;
    };

    MCP4728(i2c_bus& bus, std::uint8_t address);

    [[nodiscard]] auto present() -> bool override;
    [[nodiscard]] auto set_voltage(unsigned int channel, float voltage) -> bool;
    [[nodiscard]] auto store_settings() -> bool;
    [[nodiscard]] auto write_channel(std::uint8_t channel, const DacChannel& channelData) -> bool;
    [[nodiscard]] auto read_channel(std::uint8_t channel, bool eeprom = false)
        -> std::optional<DacChannel>;
    [[nodiscard]] auto set_vref(unsigned int channel, CFG_VREF vref_setting) -> bool;
    [[nodiscard]] auto set_vref(CFG_VREF vref_setting) -> bool;

    static auto code2voltage(const DacChannel& channelData) -> float;

    [[nodiscard]] auto identify() -> bool override;

private:
    static constexpr float fVddRefVoltage {3.3}; ///< voltage at which the device is powered
    enum COMMAND : std::uint8_t
    {
        DAC_FAST_WRITE       = 0b00000000,
        DAC_MULTI_WRITE      = 0b00001000,
        DAC_EEP_SEQ_WRITE    = 0b00001010,
        DAC_EEP_SINGLE_WRITE = 0b00001011,
        ADDR_BITS_WRITE      = 0b00001100,
        VREF_WRITE           = 0b00010000,
        GAIN_WRITE           = 0b00011000,
        PD_WRITE             = 0b00010100
    };
    std::array<DacChannel, 4>                          fChannelSetting {};
    std::array<DacChannel, 4>                          fChannelSettingEep {};
    std::chrono::time_point<std::chrono::steady_clock> fLastRegisterUpdate {};
    bool                                               fBusy {false};

    auto set_voltage(std::uint8_t channel, float voltage, bool toEEPROM) -> bool;
    auto set_value(std::uint8_t  channel,
                   std::uint16_t value,
                   CFG_GAIN      gain     = GAIN1,
                   bool          toEEPROM = false) -> bool;
    auto read_registers() -> bool;
    void parse_channel_data(const std::uint8_t* buf);
    void dump_registers();
    auto waitEepReady() -> bool;
};

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_MCP4728_H
