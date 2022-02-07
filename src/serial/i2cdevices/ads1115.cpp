#include "muonpi/serial/i2cdevices/ads1115.h"
#include <future>
#include <iomanip>
#include <iostream>

namespace muonpi::serial::devices {

/*
 * ADS1115 4ch 16 bit ADC
 */
constexpr std::uint16_t HI_RANGE_LIMIT { static_cast<std::uint16_t>(ADS1115::MAX_ADC_VALUE * 0.8) };
constexpr std::uint16_t LO_RANGE_LIMIT { static_cast<std::uint16_t>(ADS1115::MAX_ADC_VALUE * 0.2) };

auto ADS1115::Sample::operator==(const Sample& other) const -> bool
{
    return (value == other.value && voltage == other.voltage && channel == other.channel && timestamp == other.timestamp);
}

auto ADS1115::Sample::operator!=(const Sample& other) const -> bool
{
    return (!(*this == other));
}

auto ADS1115::adcToVoltage(std::int16_t adc, const CFG_PGA pga_setting) -> float
{
    return (adc * lsb_voltage(pga_setting));
}

ADS1115::ADS1115(i2c_bus& bus, std::uint8_t address) noexcept
    : i2c_device(bus, address)
{
    set_name("ADS1115");
    init();
}

ADS1115::~ADS1115()
    = default;

void ADS1115::init() noexcept
{
    m_rate = CFG_RATES::SPS8;
}

void ADS1115::setPga(CFG_PGA pga)
{
    m_pga[0] = m_pga[1] = m_pga[2] = m_pga[3] = pga;
}

void ADS1115::setPga(unsigned int pga)
{
    setPga(static_cast<CFG_PGA>(pga));
}

void ADS1115::setPga(const std::uint8_t channel, std::uint8_t pga)
{
    setPga(channel, static_cast<CFG_PGA>(pga));
}

auto ADS1115::getPga(const std::uint8_t ch) const -> CFG_PGA
{
    return m_pga.at(ch);
}

void ADS1115::setRate(unsigned int rate)
{
    m_rate = rate & 0x07u;
}

auto ADS1115::getRate() const -> unsigned int
{
    return m_rate;
}

void ADS1115::setPga(const std::uint8_t channel, CFG_PGA pga)
{
    m_pga.at(channel) = pga;
}

void ADS1115::setActiveChannel(const std::uint8_t channel, bool differential_mode)
{
    m_selected_channel = channel;
    m_diff_mode = differential_mode;
}

auto ADS1115::setContinuousSampling(bool cont_sampling) -> bool
{
    m_conv_mode = (cont_sampling) ? CONV_MODE::CONTINUOUS : CONV_MODE::SINGLE;
    return writeConfig();
}

auto ADS1115::writeConfig(bool startNewConversion) -> bool
{
    std::uint16_t conf_reg { 0 };

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
    conf_reg |= (static_cast<std::uint8_t>(m_pga.at(m_selected_channel)) & 0x07u) << 9u; // PGA gain select

    // This sets the 8 LSBs of the config register (bits 7-0)
    //conf_reg |= 0x00; // TODO: enable ALERT/RDY pin
    conf_reg |= (m_rate & 0x07u) << 5u;

    if (write(static_cast<std::uint8_t>(REG::CONFIG), &conf_reg) != 1) {
        return false;
    }
    m_current_channel = m_selected_channel;
    return true;
}

auto ADS1115::wait_conversion_finished() -> bool
{
    // Wait for the conversion to complete, this requires bit 15 to change from 0->1
    const std::size_t n_max { static_cast<std::size_t>(1'000'000UL / m_poll_period.count()) };

    for (std::size_t i { 0 }; i < n_max; i++) {
        std::uint16_t conf_reg { 0 };
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

auto ADS1115::readConversionResult(std::int16_t& dataword) -> bool
{
    std::uint16_t data { 0 };
    // Read the contents of the conversion register into readBuf
    if (read(static_cast<std::uint8_t>(REG::CONVERSION), &data) != 1) {
        return false;
    }

    dataword = static_cast<std::int16_t>(data);

    return true;
}

auto ADS1115::getSample(const std::uint8_t channel) -> ADS1115::Sample
{
    // if ( fConvMode != CONV_MODE::SINGLE ) return InvalidSample;
    std::lock_guard<std::mutex> lock(m_mutex);

    m_conv_mode = CONV_MODE::SINGLE;
    m_selected_channel = channel;

    scope_guard timer_guard { setup_timer() };

    // Write the current config to the ADS1115
    // and begin a single conversion
    if (!writeConfig(true)) {
        return InvalidSample;
    }

    if (!wait_conversion_finished()) {
        return InvalidSample;
    }

    std::int16_t conv_result { 0 }; // Stores the 16 bit value of our ADC conversion

    if (!readConversionResult(conv_result)) {
        return InvalidSample;
    }

    return generate_sample(conv_result);
}

auto ADS1115::triggerConversion(const std::uint8_t channel) -> bool
{
    // triggering a conversion makes only sense in single shot mode
    if (m_conv_mode != CONV_MODE::SINGLE) {
        return false;
    }
    try {
        auto future {std::async(std::launch::async, [&]{
            getSample(channel);
        })};
        return future.valid();
    } catch (...) {
        return false;
    }
}

auto ADS1115::conversionFinished() -> ADS1115::Sample
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::int16_t conv_result { 0 }; // Stores the 16 bit value of our ADC conversion

    if (!readConversionResult(conv_result)) {
        return InvalidSample;
    }

    stop_timer();
    start_timer();

    return generate_sample(conv_result);
}

auto ADS1115::readADC(const std::uint8_t channel) -> std::int16_t
{
    try {
        std::future<Sample> sample_future = std::async(&ADS1115::getSample, this, channel);
        sample_future.wait();
        if (sample_future.valid()) {
            Sample sample { sample_future.get() };
            if (sample != InvalidSample) {
                return sample.value;
            }
        }
    } catch (...) {
    }
    return INT16_MIN;
}

auto ADS1115::setDataReadyPinMode() -> bool
{
    // c.f. datasheet, par. 9.3.8, p. 19
    // set MSB of Lo_thresh reg to 0
    // set MSB of Hi_thresh reg to 1
    // set COMP_QUE[1:0] to any value other than '11' (default value)
    bool ok = setThreshold<REG::LO_THRESH>(static_cast<std::int16_t>(0b0000000000000000));
    ok = ok && setThreshold<REG::HI_THRESH>(static_cast<std::int16_t>(0b1111111111111111));
    ok = ok && setCompQueue(0x00u);
    return ok;
}

auto ADS1115::getReadWaitDelay() const -> unsigned int
{
    return m_poll_period.count();
}

void ADS1115::registerConversionReadyCallback(std::function<void(Sample)> fn)
{
    m_conv_ready_fn = std::move(fn);
}

auto ADS1115::setCompQueue(std::uint8_t bitpattern) -> bool
{
    std::uint16_t conf_reg { 0 };
    if (read(static_cast<std::uint8_t>(REG::CONFIG), &conf_reg) != 1) {
        return false;
    }
    conf_reg &= 0b11111100u;
    conf_reg |= bitpattern & 0b00000011u;
    return (write(static_cast<std::uint8_t>(REG::CONFIG), &conf_reg) == 1);
}

auto ADS1115::identify() -> bool
{
    if (flag_set(Flags::Failed)) {
        return false;
    }
    if (!present()) {
        return false;
    }

    std::uint16_t dataword { 0 };
    if (read(static_cast<std::uint8_t>(REG::CONFIG), &dataword) == 0) {
        return false;
    }
    if (((dataword & 0x8000u) == 0) && ((dataword & 0x0100u) != 0)) {
        return false;
    }
    std::uint16_t dataword2 { 0 };
    // try to read at addr conf_reg+4 and compare with the previously read config register
    // both should be identical since only the 2 LSBs of the pointer register are evaluated by the ADS1115
    if (read(static_cast<std::uint8_t>(REG::CONFIG) | 0x04u, &dataword2) == 0) {
        return false;
    }
    if (dataword != dataword2) {
        return false;
    }

    return true;
}

auto ADS1115::getVoltage(const std::uint8_t channel) -> double
{
    double voltage {};
    getVoltage(channel, voltage);
    return voltage;
}

void ADS1115::getVoltage(const std::uint8_t channel, double& voltage)
{
    std::int16_t adc { 0 };
    getVoltage(channel, adc, voltage);
}

void ADS1115::getVoltage(const std::uint8_t channel, std::int16_t& adc, double& voltage)
{
    Sample sample = getSample(channel);
    adc = sample.value;
    voltage = sample.voltage;
}

void ADS1115::setAGC(bool state)
{
    m_agc[0] = m_agc[1] = m_agc[2] = m_agc[3] = state;
}

void ADS1115::setAGC(std::uint8_t channel, bool state)
{
    m_agc.at(channel) = state;
}

auto ADS1115::getAGC(std::uint8_t channel) const -> bool
{
    return m_agc.at(channel);
}

auto ADS1115::generate_sample(std::int16_t conv_result) -> Sample
{
    Sample sample {
        std::chrono::steady_clock::now(),
        conv_result,
        adcToVoltage(conv_result, m_pga.at(m_current_channel)),
        lsb_voltage(m_pga.at(m_current_channel)),
        m_current_channel
    };
    if (m_conv_ready_fn && sample != InvalidSample) {
        m_conv_ready_fn(sample);
    }

    if (m_agc.at(m_current_channel)) {
        int eadc = std::abs(conv_result);
        if (eadc > HI_RANGE_LIMIT && m_pga.at(m_current_channel) > PGA6V) {
            m_pga.at(m_current_channel) = static_cast<CFG_PGA>(m_pga.at(m_current_channel) - 1);
        } else if (eadc < LO_RANGE_LIMIT && m_pga.at(m_current_channel) < PGA256MV) {
            m_pga.at(m_current_channel) = static_cast<CFG_PGA>(m_pga.at(m_current_channel) + 1);
        }
    }
    m_last_sample.at(m_current_channel) = sample;
    return sample;
}

} // namespace muonpi::serial::devices
