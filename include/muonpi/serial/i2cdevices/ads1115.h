#ifndef _ADS1115_H_
#define _ADS1115_H_

#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"

#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>

namespace muonpi::serial::devices {

// ADC ADS1x13/4/5 sampling readout delay
constexpr unsigned int READ_WAIT_DELAY_US_INIT { 10 };

/* ADS1115: 4(2) ch, 16 bit ADC  */

class ADS1115 : public i2c_device {
public:
    struct Sample {
        std::chrono::time_point<std::chrono::steady_clock> timestamp;
        int value;
        float voltage;
        float lsb_voltage;
        unsigned int channel;
        [[nodiscard]] auto operator==(const Sample& other) -> bool;
        [[nodiscard]] auto operator!=(const Sample& other) -> bool;
    };
    static constexpr Sample InvalidSample { std::chrono::steady_clock::time_point::min(), 0, 0., 0., 0 };

    using SampleCallbackType = std::function<void(Sample)>;

    static constexpr int16_t MIN_ADC_VALUE { -32768 };
    static constexpr int16_t MAX_ADC_VALUE { 32767 };
    static constexpr uint16_t FULL_SCALE_RANGE { 65535 };

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

    static auto adcToVoltage(int16_t adc, const CFG_PGA pga_setting) -> float;

    explicit ADS1115(i2c_bus& bus, std::uint8_t address);
    ~ADS1115() override;

    [[nodiscard]] auto identify() -> bool override;

    void setActiveChannel(uint8_t channel, bool differential_mode = false);
    void setPga(CFG_PGA pga) { fPga[0] = fPga[1] = fPga[2] = fPga[3] = pga; } // TODO: Don't implement methods in the header unless necessary.
    void setPga(unsigned int pga) { setPga((CFG_PGA)pga); } // TODO: Don't implement methods in the header unless necessary.
    void setPga(uint8_t channel, CFG_PGA pga);
    void setPga(uint8_t channel, uint8_t pga) { setPga(channel, (CFG_PGA)pga); } // TODO: Don't implement methods in the header unless necessary.
    [[nodiscard]] auto getPga(int ch) const -> CFG_PGA { return fPga[ch]; } // TODO: Don't implement methods in the header unless necessary.
    void setAGC(bool state);
    void setAGC(uint8_t channel, bool state);
    [[nodiscard]] auto getAGC(uint8_t channel) const -> bool;
    void setRate(unsigned int rate) { fRate = rate & 0x07; } // TODO: Don't implement methods in the header unless necessary.
    [[nodiscard]] auto getRate() const -> unsigned int { return fRate; } // TODO: Don't implement methods in the header unless necessary.
    auto setLowThreshold(int16_t thr) -> bool;
    auto setHighThreshold(int16_t thr) -> bool;
    auto readADC(unsigned int channel) -> int16_t;
    auto getVoltage(unsigned int channel) -> double;
    void getVoltage(unsigned int channel, double& voltage);
    void getVoltage(unsigned int channel, int16_t& adc, double& voltage);
    void setDiffMode(bool mode) { fDiffMode = mode; }
    auto setDataReadyPinMode() -> bool;
    [[nodiscard]] auto getReadWaitDelay() const -> unsigned int { return fReadWaitDelay; } // TODO: Don't implement methods in the header unless necessary.
    auto setContinuousSampling(bool cont_sampling = true) -> bool;
    auto triggerConversion(unsigned int channel) -> bool;
    auto getSample(unsigned int channel) -> Sample;
    auto conversionFinished() -> Sample;
    void registerConversionReadyCallback(std::function<void(Sample)> fn) { fConvReadyFn = std::move(fn); } // TODO: Don't implement methods in the header unless necessary.

protected:
    CFG_PGA fPga[4] { PGA4V, PGA4V, PGA4V, PGA4V };
    uint8_t fRate { 0x00 };
    uint8_t fCurrentChannel { 0 };
    uint8_t fSelectedChannel { 0 };
    unsigned int fReadWaitDelay { READ_WAIT_DELAY_US_INIT }; ///< conversion wait time in us
    bool fAGC[4] { false, false, false, false }; ///< software agc which switches over to a better pga setting if voltage too low/high
    bool fDiffMode { false }; ///< measure differential input signals (true) or single ended (false=default)
    CONV_MODE fConvMode { CONV_MODE::UNKNOWN };
    Sample fLastSample[4] { InvalidSample, InvalidSample, InvalidSample, InvalidSample };

    std::mutex fMutex;

    enum REG : uint8_t {
        CONVERSION = 0x00,
        CONFIG = 0x01,
        LO_THRESH = 0x02,
        HI_THRESH = 0x03
    };

    virtual void init();
    auto writeConfig(bool startNewConversion = false) -> bool;
    auto setCompQueue(uint8_t bitpattern) -> bool;
    auto readConversionResult(int16_t& dataword) -> bool;
    static constexpr auto lsb_voltage(const CFG_PGA pga_setting) -> float { return (PGAGAINS[pga_setting] / MAX_ADC_VALUE); }
    void waitConversionFinished(bool& error);
    std::function<void(Sample)> fConvReadyFn {};

private:
    static constexpr float PGAGAINS[8] { 6.144, 4.096, 2.048, 1.024, 0.512, 0.256, 0.256, 0.256 };
};

} // namespace muonpi::serial::devices
#endif // !_ADS1115_H_
