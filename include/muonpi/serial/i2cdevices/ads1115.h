#ifndef MUONPI_SERIAL_I2CDEVICES_ADS1115_H
#define MUONPI_SERIAL_I2CDEVICES_ADS1115_H

#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"

#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>

namespace muonpi::serial::devices {

// ADC ADS1x13/4/5 initial polling readout period
constexpr std::chrono::microseconds READ_WAIT_DELAY_INIT { 10 };

/* ADS1115: 4(2) ch, 16 bit ADC  */

class ADS1115 : public i2c_device {
public:
    struct Sample {
        std::chrono::time_point<std::chrono::steady_clock> timestamp;
        int value;
        float voltage;
        float lsb_voltage;
        unsigned int channel;
        [[nodiscard]] auto operator==(const Sample& other) const -> bool;
        [[nodiscard]] auto operator!=(const Sample& other) const -> bool;
    };
    static constexpr Sample InvalidSample { std::chrono::steady_clock::time_point::min(), 0, 0., 0., 0 };

    using SampleCallbackType = std::function<void(Sample)>;

    static constexpr std::int16_t MIN_ADC_VALUE { -32768 };
    static constexpr std::int16_t MAX_ADC_VALUE { 32767 };
    static constexpr std::uint16_t FULL_SCALE_RANGE { 65535 };

    enum CFG_CHANNEL { CH0 = 0,
        CH1,
        CH2,
        CH3 };
    enum CFG_DIFF_CHANNEL { CH0_1 = 0,
        CH0_3,
        CH1_3,
        CH2_3 };
    enum CFG_PGA { PGA6V = 0,
        PGA4V = 1,
        PGA2V = 2,
        PGA1V = 3,
        PGA512MV = 4,
        PGA256MV = 5 };
    enum CFG_RATES { SPS8 = 0x00,
        SPS16 = 0x01,
        SPS32 = 0x02,
        SPS64 = 0x03,
        SPS128 = 0x04,
        SPS250 = 0x05,
        SPS475 = 0x06,
        SPS860 = 0x07 };
    enum class CONV_MODE { UNKNOWN,
        SINGLE,
        CONTINUOUS };

    enum REG : std::uint8_t {
        CONVERSION = 0x00,
        CONFIG = 0x01,
        LO_THRESH = 0x02,
        HI_THRESH = 0x03
    };

    static auto adcToVoltage(std::int16_t adc, CFG_PGA pga_setting) -> float;

    explicit ADS1115(i2c_bus& bus, std::uint8_t address);
    ~ADS1115() override;

    [[nodiscard]] auto identify() -> bool override;

    void setActiveChannel(std::uint8_t channel, bool differential_mode = false);
    void setPga(CFG_PGA pga);
    void setPga(unsigned int pga);
    void setPga(std::uint8_t channel, CFG_PGA pga);
    void setPga(std::uint8_t channel, std::uint8_t pga);
    [[nodiscard]] auto getPga(int ch) const -> CFG_PGA;
    void setAGC(bool state);
    void setAGC(std::uint8_t channel, bool state);
    [[nodiscard]] auto getAGC(std::uint8_t channel) const -> bool;
    void setRate(unsigned int rate);
    [[nodiscard]] auto getRate() const -> unsigned int;

    template <REG R>
    auto setThreshold(std::int16_t threshold) -> bool;

    auto readADC(unsigned int channel) -> std::int16_t;
    auto getVoltage(unsigned int channel) -> double;
    void getVoltage(unsigned int channel, double& voltage);
    void getVoltage(unsigned int channel, std::int16_t& adc, double& voltage);
    void setDiffMode(bool mode) { m_diff_mode = mode; }
    auto setDataReadyPinMode() -> bool;
    [[nodiscard]] auto getReadWaitDelay() const -> unsigned int;
    auto setContinuousSampling(bool cont_sampling = true) -> bool;
    auto triggerConversion(unsigned int channel) -> bool;
    auto getSample(unsigned int channel) -> Sample;
    auto conversionFinished() -> Sample;
    void registerConversionReadyCallback(std::function<void(Sample)> fn);

protected:
    CFG_PGA m_pga[4] { PGA4V, PGA4V, PGA4V, PGA4V };
    std::uint8_t m_rate { 0x00 };
    std::uint8_t m_current_channel { 0 };
    std::uint8_t m_selected_channel { 0 };
    std::chrono::microseconds m_poll_period { READ_WAIT_DELAY_INIT }; ///< conversion wait time in us
    bool m_agc[4] { false, false, false, false }; ///< software agc which switches over to a better pga setting if voltage too low/high
    bool m_diff_mode { false }; ///< measure differential input signals (true) or single ended (false=default)
    CONV_MODE m_conv_mode { CONV_MODE::UNKNOWN };
    Sample m_last_sample[4] { InvalidSample, InvalidSample, InvalidSample, InvalidSample };

    std::mutex m_mutex;

    virtual void init();
    auto writeConfig(bool startNewConversion = false) -> bool;
    auto setCompQueue(std::uint8_t bitpattern) -> bool;
    auto readConversionResult(std::int16_t& dataword) -> bool;
    static constexpr auto lsb_voltage(const CFG_PGA pga_setting) -> float { return (PGAGAINS[pga_setting] / MAX_ADC_VALUE); }

    /**
     * @brief wait_conversion_finished polls for the conversion to be done.
     * This is indicated by bit 15 of the config register to change from 0 to 1.
     * Polls in discrete time intervals of m_poll_period
     * @return false in case of timeout or read failure.
     */
    [[nodiscard]] auto wait_conversion_finished() -> bool;

    std::function<void(Sample)> m_conv_ready_fn {};

private:
    [[nodiscard]] auto generate_sample(std::int16_t conv_result) -> Sample;

    static constexpr float PGAGAINS[8] { 6.144, 4.096, 2.048, 1.024, 0.512, 0.256, 0.256, 0.256 };
};

template <ADS1115::REG R>
auto ADS1115::setThreshold(std::int16_t threshold) -> bool
{
    static_assert((R == ADS1115::REG::LO_THRESH) || (R == ADS1115::REG::HI_THRESH), "setThreshold() of invalid register");

    start_timer();
    std::uint16_t reg_content { static_cast<std::uint16_t>(threshold) };
    if (write(static_cast<std::uint8_t>(R), &reg_content) != 1) {
        return false;
    }

    reg_content = { 0 };
    // Read back the contents of the threshold register
    if (read(static_cast<std::uint8_t>(R), &reg_content) != 1) {
        return false;
    }
    std::int16_t readback_value { static_cast<std::int16_t>(reg_content) };
    if (readback_value != threshold) {
        return false;
    }
    stop_timer();
    return true;
}

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_ADS1115_H
