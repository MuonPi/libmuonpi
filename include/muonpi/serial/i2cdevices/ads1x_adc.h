#ifndef MUONPI_SERIAL_I2CDEVICES_ADS1X_H
#define MUONPI_SERIAL_I2CDEVICES_ADS1X_H

#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"
#include "muonpi/scopeguard.h"

#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>
#include <array>
#include <future>
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t

namespace muonpi::serial::devices {

template <int CHANNELS=4, int BITS=16, bool PGA=true>
class ADS1X_ADC : public i2c_device {
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

    static constexpr std::int16_t MIN_ADC_VALUE { -(1 << BITS-1) };
    static constexpr std::int16_t MAX_ADC_VALUE { (1 << BITS-1) - 1 };
    static constexpr std::uint16_t FULL_SCALE_RANGE { (1 << BITS) - 1 };

    struct pga_t {
        enum { 
            PgaMin = 0,
            PGA6V = PgaMin, PGA4V = 1, PGA2V = 2, PGA1V = 3, PGA512MV = 4, PGA256MV = 5, PgaMax = PGA256MV 
        };
        constexpr pga_t() noexcept = default;
        
        template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        /**
        * @brief pga_t construct a pga_t object. Explicitly not marked explicit.
        * @param a_pga The pga setting to represent
        */
        constexpr pga_t(T a_pga) noexcept
        : m_pga { std::clamp(static_cast<int>(a_pga), static_cast<int>(PgaMin), static_cast<int>(PgaMax)) } {}

        template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        /**
        * @brief operator = assign a value to the PGA setting
        * @param other
        * @return
        */
        constexpr auto operator=(T other) noexcept -> const pga_t&;

        template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        /**
        * @brief operator T Convert the PGA object to an integral type
        */
        [[nodiscard]] constexpr explicit operator T() const noexcept;
        
        // Prefix increment
        pga_t& operator++() { m_pga = std::min( m_pga+1, static_cast<int>(PgaMax) ); return *this; }  

        // Postfix increment
        pga_t operator++(int) { pga_t tmp = *this; ++(*this); return tmp; }

        // Prefix decrement
        pga_t& operator--() { m_pga = std::max( m_pga-1, static_cast<int>(PgaMin) ); return *this; }  

        // Postfix decrement
        pga_t operator--(int) { pga_t tmp = *this; --(*this); return tmp; }

        static constexpr std::array<float, 8> gain_values { 6.144, 4.096, 2.048, 1.024, 0.512, 0.256, 0.256, 0.256 };
    private:
        int m_pga { PgaMin };
    };

    struct sample_rate_t {
        enum {
            RateMin = 0x00,
            Rate0 = RateMin, Rate1 = 0x01, Rate2 = 0x02, Rate3 = 0x03,
            Rate4 = 0x04, Rate5 = 0x05, Rate6 = 0x06, Rate7 = 0x07, RateMax = Rate7
        };
        constexpr sample_rate_t() noexcept = default;
        template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        /**
        * @brief sample_rate_t construct a sample_rate_t object. Explicitly not marked explicit.
        * @param a_rate_setting The sample rate setting to represent
        */
        constexpr sample_rate_t(T a_rate_setting) noexcept
        : m_setting {std::clamp(static_cast<int>(a_rate_setting), static_cast<int>(RateMin), static_cast<int>(RateMax))} {}

        template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        /**
        * @brief operator = assign a value to the sample rate setting
        * @param other
        * @return
        */
        constexpr auto operator=(T other) noexcept -> const sample_rate_t&;

        template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        /**
        * @brief operator T Convert the sample rate setting object to an integral type
        */
        [[nodiscard]] constexpr explicit operator T() const noexcept;
        
        template <int TYPE = BITS, std::enable_if_t<TYPE==16 || TYPE==12, bool> = true>
        static constexpr std::array<std::size_t,8> rate_values = { sample_rate_t::sample_rates(TYPE) };

    private:
        int m_setting { RateMin };
        static constexpr std::array<std::size_t,8> sample_rates( int bit_width ) {
            if ( bit_width == 16 ) {
                return { 8,16,32,64,128,250,475,860 };
            }
            return { 128,250,490,920,1600,2400,3300,3300 };
        }
    };

    enum class CONV_MODE { UNKNOWN,
        SINGLE,
        CONTINUOUS };

    enum REG : std::uint8_t {
        CONVERSION = 0x00,
        CONFIG = 0x01,
        LO_THRESH = 0x02,
        HI_THRESH = 0x03
    };

    static auto adcToVoltage(std::int16_t adc, pga_t pga_setting) -> float;

    ADS1X_ADC(i2c_bus& bus, std::uint8_t address) noexcept;
    ~ADS1X_ADC() override = default;

    [[nodiscard]] auto identify() -> bool override;

    void setActiveChannel(std::uint8_t channel, bool differential_mode = false);
    void setPga(pga_t pga);
    void setPga(const std::uint8_t channel, pga_t pga);
    [[nodiscard]] auto getPga(const std::uint8_t ch) const -> pga_t;
    void setAGC(bool state);
    void setAGC(const std::uint8_t channel, bool state);
    [[nodiscard]] auto getAGC(const std::uint8_t channel) const -> bool;
    void setRate(unsigned int rate);
    [[nodiscard]] auto getRate() const -> unsigned int;

    template <REG R>
    auto setThreshold(std::int16_t threshold) -> bool;

    auto readADC(const std::uint8_t channel) -> std::int16_t;
    auto getVoltage(const std::uint8_t channel) -> double;
    void getVoltage(const std::uint8_t channel, double& voltage);
    void getVoltage(const std::uint8_t channel, std::int16_t& adc, double& voltage);
    void setDiffMode(bool mode) { m_diff_mode = mode; }
    auto setDataReadyPinMode() -> bool;
    [[nodiscard]] auto getReadWaitDelay() const -> unsigned int;
    auto setContinuousSampling(bool cont_sampling = true) -> bool;
    auto triggerConversion(const std::uint8_t channel) -> bool;
    auto getSample(const std::uint8_t channel) -> Sample;
    auto conversionFinished() -> Sample;
    void registerConversionReadyCallback(std::function<void(Sample)> fn);

protected:
    std::array<pga_t,CHANNELS> m_pga { static_cast<int>(pga_t::PGA4V) };
    sample_rate_t m_rate { static_cast<int>(sample_rate_t::Rate0) };
    std::uint8_t m_current_channel { 0 };
    std::uint8_t m_selected_channel { 0 };
    std::chrono::microseconds m_poll_period { READ_WAIT_DELAY_INIT }; ///< conversion wait time in us
    std::array<bool,CHANNELS> m_agc { false }; ///< software agc which switches over to a better pga setting if voltage too low/high
    bool m_diff_mode { false }; ///< measure differential input signals (true) or single ended (false=default)
    CONV_MODE m_conv_mode { CONV_MODE::UNKNOWN };
    std::array<Sample,CHANNELS> m_last_sample { InvalidSample };

    std::mutex m_mutex;

    void init() noexcept;
    auto writeConfig(bool startNewConversion = false) -> bool;
    auto setCompQueue(std::uint8_t bitpattern) -> bool;
    auto readConversionResult(std::int16_t& dataword) -> bool;
    [[nodiscard]] static constexpr auto lsb_voltage(const pga_t pga_setting) -> float { 
        return (pga_t::gain_values[static_cast<int>(pga_setting)] / MAX_ADC_VALUE); 
    }

    /**
     * @brief wait_conversion_finished polls for the conversion to be done.
     * This is indicated by bit 15 of the config register to change from 0 to 1.
     * Polls in discrete time intervals of m_poll_period
     * @return false in case of timeout or read failure.
     */
    [[nodiscard]] auto wait_conversion_finished() -> bool;

    std::function<void(Sample)> m_conv_ready_fn {};

private:
    static constexpr std::chrono::microseconds READ_WAIT_DELAY_INIT { 10 }; ///<! ADC ADS1x13/4/5 initial polling readout period
    static constexpr std::uint16_t HI_RANGE_LIMIT {static_cast<std::uint16_t>(MAX_ADC_VALUE * 0.8)};
    static constexpr std::uint16_t LO_RANGE_LIMIT {static_cast<std::uint16_t>(MAX_ADC_VALUE * 0.2)};
    [[nodiscard]] auto generate_sample(std::int16_t conv_result) -> Sample;
};

/*********************
 * Implementation part
 *********************/
template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::Sample::operator==(const Sample& other) const -> bool {
    return (value == other.value && voltage == other.voltage && channel == other.channel
            && timestamp == other.timestamp);
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::Sample::operator!=(const Sample& other) const -> bool {
    return (!(*this == other));
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::adcToVoltage(std::int16_t adc, pga_t pga_setting) -> float {
    return (adc * lsb_voltage(pga_setting));
}

template <int CHANNELS, int BITS, bool PGA>
template <typename T, std::enable_if_t<std::is_integral<T>::value, bool>>
constexpr auto ADS1X_ADC<CHANNELS,BITS,PGA>::pga_t::operator=(T other) noexcept -> const pga_t& {
    m_pga = std::clamp(static_cast<int>(other), static_cast<int>(PgaMin), static_cast<int>(PgaMax) );
    return *this;
}

template <int CHANNELS, int BITS, bool PGA>
template <typename T, std::enable_if_t<std::is_integral<T>::value, bool>>
constexpr ADS1X_ADC<CHANNELS,BITS,PGA>::pga_t::operator T() const noexcept {
    return static_cast<T>(m_pga);
}

template <int CHANNELS, int BITS, bool PGA>
template <typename T, std::enable_if_t<std::is_integral<T>::value, bool>>
constexpr auto ADS1X_ADC<CHANNELS,BITS,PGA>::sample_rate_t::operator=(T other) noexcept -> const sample_rate_t& {
    m_setting = std::clamp(static_cast<int>(other), static_cast<int>(RateMin), static_cast<int>(RateMax) );
    return *this;
}

template <int CHANNELS, int BITS, bool PGA>
template <typename T, std::enable_if_t<std::is_integral<T>::value, bool>>
constexpr ADS1X_ADC<CHANNELS,BITS,PGA>::sample_rate_t::operator T() const noexcept {
    return static_cast<T>(m_setting);
}

template <int CHANNELS, int BITS, bool PGA>
ADS1X_ADC<CHANNELS,BITS,PGA>::ADS1X_ADC(i2c_bus& bus, std::uint8_t address) noexcept
    : i2c_device(bus, address) 
{
    static_assert( CHANNELS == 1 || CHANNELS == 4,
                  "illegal number of channels (must be 1 or 4)");
    static_assert( BITS == 12 || BITS == 16,
                  "illegal number of bits (must be 12 or 16)");
    
    std::string typestr { "ADS1" };
    if ( BITS == 16 ) {
        typestr += "11";
    } else {
        typestr += "01";
    }        
        
    if ( CHANNELS == 4 ) {
        typestr += '5';
    } else if ( PGA ) {
        typestr += '4';
    } else {
        typestr += '3';
    }
        
    set_name( name() + "::" + typestr );
    m_addresses_hint = {0x48, 0x49, 0x4a, 0x4b};
    init();
}

template <int CHANNELS, int BITS, bool PGA>
void ADS1X_ADC<CHANNELS,BITS,PGA>::init() noexcept {
    m_rate = static_cast<int>(sample_rate_t::Rate0);
}

template <int CHANNELS, int BITS, bool PGA>
void ADS1X_ADC<CHANNELS,BITS,PGA>::setPga(pga_t pga) {
    m_pga[0] = m_pga[1] = m_pga[2] = m_pga[3] = pga;
}

template <int CHANNELS, int BITS, bool PGA>
void ADS1X_ADC<CHANNELS,BITS,PGA>::setPga(const std::uint8_t channel, pga_t pga) {
    m_pga.at(channel) = pga;
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::getPga(const std::uint8_t ch) const -> pga_t {
    return m_pga.at(ch);
}

template <int CHANNELS, int BITS, bool PGA>
void ADS1X_ADC<CHANNELS,BITS,PGA>::setRate(unsigned int rate) {
    m_rate = static_cast<sample_rate_t>( rate & 0x07u );
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::getRate() const -> unsigned int {
    return m_rate;
}

template <int CHANNELS, int BITS, bool PGA>
void ADS1X_ADC<CHANNELS,BITS,PGA>::setActiveChannel(const std::uint8_t channel, bool differential_mode) {
    static_assert(CHANNELS > 1, "setting active channel not allowed on single channel device");
    m_selected_channel = channel;
    m_diff_mode        = differential_mode;
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::setContinuousSampling(bool cont_sampling) -> bool {
    m_conv_mode = (cont_sampling) ? CONV_MODE::CONTINUOUS : CONV_MODE::SINGLE;
    return writeConfig();
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::writeConfig(bool startNewConversion) -> bool {
    std::uint16_t conf_reg {0};

    // read in the current contents of config reg only if conv_mode is unknown
    if (m_conv_mode == CONV_MODE::UNKNOWN) {
        if (read(static_cast<std::uint8_t>(REG::CONFIG), &conf_reg) != 1) {
            return false;
        }
        if ((conf_reg & 0x0100u) == 0u) {
            m_conv_mode = CONV_MODE::CONTINUOUS;
        } else {
            m_conv_mode = CONV_MODE::SINGLE;
        }
    }

    conf_reg = 0;

    if (m_conv_mode == CONV_MODE::SINGLE && startNewConversion) {
        conf_reg = 0x8000u; // set OS bit
    }
    if (!m_diff_mode) {
        conf_reg |= 0x4000u; // single ended mode channels
    }
    conf_reg |= (m_selected_channel & 0x03u) << 12u; // channel select
    if (m_conv_mode == CONV_MODE::SINGLE) {
        conf_reg |= 0x0100u; // single shot mode
    }
    conf_reg |= (static_cast<std::uint8_t>(m_pga.at(m_selected_channel)) & 0x07u)
             << 9u; // PGA gain select

    // This sets the 8 LSBs of the config register (bits 7-0)
    // conf_reg |= 0x00; // TODO: enable ALERT/RDY pin
    conf_reg |= (static_cast<std::uint8_t>(m_rate) & 0x07u) << 5u;

    if (write(static_cast<std::uint8_t>(REG::CONFIG), &conf_reg) != 1) {
        return false;
    }
    m_current_channel = m_selected_channel;
    return true;
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::wait_conversion_finished() -> bool {
    // Wait for the conversion to complete, this requires bit 15 to change from 0->1
    const std::size_t n_max {static_cast<std::size_t>(1'000'000UL / m_poll_period.count())};

    for (std::size_t i {0}; i < n_max; i++) {
        std::uint16_t conf_reg {0};
        std::this_thread::sleep_for(m_poll_period);
        if (read(static_cast<std::uint8_t>(REG::CONFIG), &conf_reg) != 1) {
            return false;
        }
        if ((conf_reg & 0x8000u) > 0) {
            if (i > 1) {
                m_poll_period += std::chrono::microseconds((i - 1) * m_poll_period.count() / 10);
            }
            return true;
        }
    }
    return false;
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::readConversionResult(std::int16_t& dataword) -> bool {
    std::uint16_t data {0};
    // Read the contents of the conversion register into readBuf
    if (read(static_cast<std::uint8_t>(REG::CONVERSION), &data) != 1) {
        return false;
    }

    if (BITS == 12) data >>= 4;
    dataword = static_cast<std::int16_t>(data);

    return true;
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::getSample(const std::uint8_t channel) -> ADS1X_ADC<CHANNELS,BITS,PGA>::Sample {
    static_assert(CHANNELS > 1 || channel == 1, "invalid channel selection for single channel device");
    std::lock_guard<std::mutex> lock(m_mutex);

    m_conv_mode        = CONV_MODE::SINGLE;
    m_selected_channel = channel;

    scope_guard timer_guard {setup_timer()};

    // Write the current config to the ADS1115
    // and begin a single conversion
    if (!writeConfig(true)) {
        return InvalidSample;
    }

    if (!wait_conversion_finished()) {
        return InvalidSample;
    }

    std::int16_t conv_result {0}; // Stores the 16 bit value of our ADC conversion

    if (!readConversionResult(conv_result)) {
        return InvalidSample;
    }

    return generate_sample(conv_result);
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::triggerConversion(const std::uint8_t channel) -> bool {
    static_assert(CHANNELS > 1 || channel == 1, "invalid channel selection for single channel device");
    // triggering a conversion makes only sense in single shot mode
    if (m_conv_mode != CONV_MODE::SINGLE) {
        return false;
    }
    try {
        auto future {std::async(std::launch::async, [&] { getSample(channel); })};
        return future.valid();
    } catch (...) { return false; }
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::conversionFinished() -> ADS1X_ADC<CHANNELS,BITS,PGA>::Sample {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::int16_t                conv_result {0}; // Stores the 16 bit value of our ADC conversion

    if (!readConversionResult(conv_result)) {
        return InvalidSample;
    }

    stop_timer();
    start_timer();

    return generate_sample(conv_result);
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::readADC(const std::uint8_t channel) -> std::int16_t {
    static_assert(CHANNELS > 1 || channel == 1, "invalid channel selection for single channel device");
    try {
        std::future<Sample> sample_future = std::async(&ADS1X_ADC<CHANNELS,BITS,PGA>::getSample, this, channel);
        sample_future.wait();
        if (sample_future.valid()) {
            Sample sample {sample_future.get()};
            if (sample != InvalidSample) {
                return sample.value;
            }
        }
    } catch (...) {}
    return INT16_MIN;
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::setDataReadyPinMode() -> bool {
    // c.f. datasheet, par. 9.3.8, p. 19
    // set MSB of Lo_thresh reg to 0
    // set MSB of Hi_thresh reg to 1
    // set COMP_QUE[1:0] to any value other than '11' (default value)
    bool ok = setThreshold<REG::LO_THRESH>(static_cast<std::int16_t>(0b0000000000000000));
    ok      = ok && setThreshold<REG::HI_THRESH>(static_cast<std::int16_t>(0b1111111111111111));
    ok      = ok && setCompQueue(0x00u);
    return ok;
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::getReadWaitDelay() const -> unsigned int {
    return m_poll_period.count();
}

template <int CHANNELS, int BITS, bool PGA>
void ADS1X_ADC<CHANNELS,BITS,PGA>::registerConversionReadyCallback(std::function<void(Sample)> fn) {
    m_conv_ready_fn = std::move(fn);
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::setCompQueue(std::uint8_t bitpattern) -> bool {
    std::uint16_t conf_reg {0};
    if (read(static_cast<std::uint8_t>(REG::CONFIG), &conf_reg) != 1) {
        return false;
    }
    conf_reg &= 0b11111100u;
    conf_reg |= bitpattern & 0b00000011u;
    return (write(static_cast<std::uint8_t>(REG::CONFIG), &conf_reg) == 1);
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::identify() -> bool {
    if (flag_set(Flags::Failed)) {
        return false;
    }
    if (!present()) {
        return false;
    }

    std::uint16_t dataword {0};
    if (read(static_cast<std::uint8_t>(REG::CONFIG), &dataword) == 0) {
        return false;
    }
    if (((dataword & 0x8000u) == 0) && ((dataword & 0x0100u) != 0)) {
        return false;
    }
    std::uint16_t dataword2 {0};
    // try to read at addr conf_reg+4 and compare with the previously read config register
    // both should be identical since only the 2 LSBs of the pointer register are evaluated by the
    // ADS1115
    if (read(static_cast<std::uint8_t>(REG::CONFIG) | 0x04u, &dataword2) == 0) {
        return false;
    }
    if (dataword != dataword2) {
        return false;
    }

    return true;
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::getVoltage(const std::uint8_t channel) -> double {
    double voltage {};
    getVoltage(channel, voltage);
    return voltage;
}

template <int CHANNELS, int BITS, bool PGA>
void ADS1X_ADC<CHANNELS,BITS,PGA>::getVoltage(const std::uint8_t channel, double& voltage) {
    std::int16_t adc {0};
    getVoltage(channel, adc, voltage);
}

template <int CHANNELS, int BITS, bool PGA>
void ADS1X_ADC<CHANNELS,BITS,PGA>::getVoltage(const std::uint8_t channel, std::int16_t& adc, double& voltage) {
    Sample sample = getSample(channel);
    adc           = sample.value;
    voltage       = sample.voltage;
}

template <int CHANNELS, int BITS, bool PGA>
void ADS1X_ADC<CHANNELS,BITS,PGA>::setAGC(bool state) {
    m_agc[0] = m_agc[1] = m_agc[2] = m_agc[3] = state;
}

template <int CHANNELS, int BITS, bool PGA>
void ADS1X_ADC<CHANNELS,BITS,PGA>::setAGC(std::uint8_t channel, bool state) {
    m_agc.at(channel) = state;
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::getAGC(std::uint8_t channel) const -> bool {
    return m_agc.at(channel);
}

template <int CHANNELS, int BITS, bool PGA>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::generate_sample(std::int16_t conv_result) -> Sample {
    Sample sample {std::chrono::steady_clock::now(),
                   conv_result,
                   adcToVoltage(conv_result, m_pga.at(m_current_channel)),
                   lsb_voltage(m_pga.at(m_current_channel)),
                   m_current_channel};
    if (m_conv_ready_fn && sample != InvalidSample) {
        m_conv_ready_fn(sample);
    }

    if (m_agc.at(m_current_channel)) {
        int eadc = std::abs(conv_result);
        if (eadc > HI_RANGE_LIMIT) {
            m_pga.at(m_current_channel)--;
        } else if (eadc < LO_RANGE_LIMIT) {
            m_pga.at(m_current_channel)--;
        }
    }
    m_last_sample.at(m_current_channel) = sample;
    return sample;
}

template <int CHANNELS, int BITS, bool PGA>
template <typename ADS1X_ADC<CHANNELS,BITS,PGA>::REG R>
auto ADS1X_ADC<CHANNELS,BITS,PGA>::setThreshold(std::int16_t threshold) -> bool
{
    static_assert((R == ADS1X_ADC<CHANNELS,BITS,PGA>::REG::LO_THRESH) || (R == ADS1X_ADC<CHANNELS,BITS,PGA>::REG::HI_THRESH), "setThreshold() of invalid register");

    scope_guard timer_guard { setup_timer() };
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
    return true;
}

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_ADS1X_H
