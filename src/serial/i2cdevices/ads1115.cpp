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

ADS1115::ADS1115(i2c_bus& bus, std::uint8_t address)
    : i2c_device(bus, address)
{
    set_title("ADS1115");
    init();
}

ADS1115::~ADS1115()
    = default;

void ADS1115::init()
{
    m_rate = CFG_RATES::SPS8;
}

void ADS1115::setPga(CFG_PGA pga)
{ 
    m_pga[0] = m_pga[1] = m_pga[2] = m_pga[3] = pga; 
}

void ADS1115::setPga(unsigned int pga) 
{ 
    setPga( static_cast<CFG_PGA>(pga) ); 
} 

void ADS1115::setPga(std::uint8_t channel, std::uint8_t pga) 
{ 
    setPga(channel, static_cast<CFG_PGA>(pga));
}

auto ADS1115::getPga(int ch) const -> CFG_PGA 
{ 
    return m_pga[ch]; 
}

void ADS1115::setRate(unsigned int rate) 
{ 
    m_rate = rate & 0x07; 
}

auto ADS1115::getRate() const -> unsigned int 
{ 
    return m_rate; 
}

void ADS1115::setPga(std::uint8_t channel, CFG_PGA pga)
{
    if (channel > 3) {
        return;
    }
    m_pga[channel] = pga;
}

void ADS1115::setActiveChannel(std::uint8_t channel, bool differential_mode)
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
        if (read(static_cast<std::uint8_t>(REG::CONFIG), &conf_reg) == 0) {
            return false;
        }
        if ((conf_reg & 0x0100) == 0) {
            m_conv_mode = CONV_MODE::CONTINUOUS;
        } else {
            m_conv_mode = CONV_MODE::SINGLE;
        }
    }

    conf_reg = 0;

    if (m_conv_mode == CONV_MODE::SINGLE && startNewConversion) {
        conf_reg = 0x8000; // set OS bit
    }
    if (!m_diff_mode) {
        conf_reg |= 0x4000; // single ended mode channels
    }
    conf_reg |= (m_selected_channel & 0x03) << 12; // channel select
    if (m_conv_mode == CONV_MODE::SINGLE) {
        conf_reg |= 0x0100; // single shot mode
    }
    conf_reg |= (static_cast<std::uint8_t>(m_pga[m_selected_channel]) & 0x07) << 9; // PGA gain select

    // This sets the 8 LSBs of the config register (bits 7-0)
    conf_reg |= 0x00; // TODO: enable ALERT/RDY pin
    conf_reg |= (m_rate & 0x07) << 5;

    if (write(static_cast<std::uint8_t>(REG::CONFIG), &conf_reg) == 0) {
        return false;
    }
    m_current_channel = m_selected_channel;
    return true;
}

void ADS1115::waitConversionFinished(bool& error)
{
    std::uint16_t conf_reg { 0 };
    // Wait for the conversion to complete, this requires bit 15 to change from 0->1
    int nloops = 0;
    while ((conf_reg & 0x8000) == 0 && nloops * m_poll_period.count() / 1000 < 1000) // readBuf[0] contains 8 MSBs of config register, AND with 10000000 to select bit 15
    {
        std::this_thread::sleep_for(m_poll_period);
        if (read(static_cast<std::uint8_t>(REG::CONFIG), &conf_reg) == 0) {
            error = true;
            return;
        }
        nloops++;
    }
    if (nloops * m_poll_period.count() / 1000 >= 1000) {
        error = true;
        return;
    }
    if (nloops > 1) {
        m_poll_period += std::chrono::microseconds( (nloops - 1) * m_poll_period.count() / 10 );
    }
    error = false;
}

auto ADS1115::readConversionResult(std::int16_t& dataword) -> bool
{
    std::uint16_t data { 0 };
    // Read the contents of the conversion register into readBuf
    if (read(static_cast<std::uint8_t>(REG::CONVERSION), &data) == 0) {
        return false;
    }

    dataword = static_cast<std::int16_t>(data);

    return true;
}

auto ADS1115::getSample(unsigned int channel) -> ADS1115::Sample
{
    // if ( fConvMode != CONV_MODE::SINGLE ) return InvalidSample;
    std::lock_guard<std::mutex> lock(m_mutex);
    std::int16_t conv_result { 0 }; // Stores the 16 bit value of our ADC conversion

    m_conv_mode = CONV_MODE::SINGLE;
    m_selected_channel = channel;

    start_timer();

    // Write the current config to the ADS1115
    // and begin a single conversion
    if (!writeConfig(true)) {
        return InvalidSample;
    }

    bool err { false };
    waitConversionFinished(err);
    if (err) {
        return InvalidSample;
    }

    if (!readConversionResult(conv_result)) {
        return InvalidSample;
    }

    stop_timer();

    Sample sample {
        std::chrono::steady_clock::now(),
        conv_result,
        adcToVoltage(conv_result, m_pga[m_current_channel]),
        lsb_voltage(m_pga[m_current_channel]),
        m_current_channel
    };
    if (m_conv_ready_fn && sample != InvalidSample) {
        m_conv_ready_fn(sample);
    }

    if (m_agc[m_current_channel]) {
        int eadc = std::abs(conv_result);
        if (eadc > HI_RANGE_LIMIT && m_pga[m_current_channel] > PGA6V) {
            m_pga[m_current_channel] = CFG_PGA(m_pga[m_current_channel] - 1);
            // if (fDebugLevel > 1) printf("ADC input high...setting PGA to level %d\n", m_pga[m_current_channel]);
        } else if (eadc < LO_RANGE_LIMIT && m_pga[m_current_channel] < PGA256MV) {
            m_pga[m_current_channel] = CFG_PGA(m_pga[m_current_channel] + 1);
            // if (fDebugLevel > 1) printf("ADC input low...setting PGA to level %d\n", m_pga[m_current_channel]);
        }
    }
    m_last_sample[m_current_channel] = sample;
    return sample;
}

auto ADS1115::triggerConversion(unsigned int channel) -> bool
{
    // triggering a conversion makes only sense in single shot mode
    if (m_conv_mode == CONV_MODE::SINGLE) {
        try {
            std::thread sampler(&ADS1115::getSample, this, channel);
            sampler.detach();
            return true;
        } catch (...) {
        }
    }
    return false;
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

    Sample sample {
        std::chrono::steady_clock::now(),
        conv_result,
        adcToVoltage(conv_result, m_pga[m_current_channel]),
        lsb_voltage(m_pga[m_current_channel]),
        m_current_channel
    };
    if (m_conv_ready_fn && sample != InvalidSample) {
        m_conv_ready_fn(sample);
    }

    if (m_agc[m_current_channel]) {
        int eadc = std::abs(conv_result);
        if (eadc > HI_RANGE_LIMIT && m_pga[m_current_channel] > PGA6V) {
            m_pga[m_current_channel] = CFG_PGA(m_pga[m_current_channel] - 1);
            // if (fDebugLevel > 1) printf("ADC input high...setting PGA to level %d\n", fPga[m_current_channel]);
        } else if (eadc < LO_RANGE_LIMIT && m_pga[m_current_channel] < PGA256MV) {
            m_pga[m_current_channel] = CFG_PGA(m_pga[m_current_channel] + 1);
            // if (fDebugLevel > 1) printf("ADC input low...setting PGA to level %d\n", fPga[m_current_channel]);
        }
    }
    m_last_sample[m_current_channel] = sample;
    return sample;
}

auto ADS1115::readADC(unsigned int channel) -> std::int16_t
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
    ok = ok && setCompQueue(0x00);
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
    conf_reg &= 0b11111100;
    conf_reg |= bitpattern & 0b00000011;
    return ( write(static_cast<std::uint8_t>(REG::CONFIG), &conf_reg) == 1 );
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
    if (((dataword & 0x8000) == 0) && ((dataword & 0x0100) != 0)) {
        return false;
    }
    std::uint16_t dataword2 { 0 };
    // try to read at addr conf_reg+4 and compare with the previously read config register
    // both should be identical since only the 2 LSBs of the pointer register are evaluated by the ADS1115
    if (read(static_cast<std::uint8_t>(REG::CONFIG) | 0x04, &dataword2) == 0) {
        return false;
    }
    if (dataword != dataword2) {
        return false;
    }

    return true;
}

auto ADS1115::getVoltage(unsigned int channel) -> double
{
    double voltage {};
    getVoltage(channel, voltage);
    return voltage;
}

void ADS1115::getVoltage(unsigned int channel, double& voltage)
{
    std::int16_t adc { 0 };
    getVoltage(channel, adc, voltage);
}

void ADS1115::getVoltage(unsigned int channel, std::int16_t& adc, double& voltage)
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
    if (channel > 3) {
        return;
    }
    m_agc[channel] = state;
}

auto ADS1115::getAGC(std::uint8_t channel) const -> bool
{
    return m_agc[channel & 0x03];
}

} // namespace muonpi::serial::devices
