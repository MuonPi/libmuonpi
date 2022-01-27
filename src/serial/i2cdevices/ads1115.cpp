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

auto ADS1115::Sample::operator!=(const Sample& other) -> bool
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
    fRate = CFG_RATES::SPS8;
}

void ADS1115::setPga(CFG_PGA pga)
{ 
    fPga[0] = fPga[1] = fPga[2] = fPga[3] = pga; 
}

void ADS1115::setPga(unsigned int pga) 
{ 
    setPga((CFG_PGA)pga); 
} 

void ADS1115::setPga(std::uint8_t channel, std::uint8_t pga) 
{ 
    setPga(channel, (CFG_PGA)pga);
}

auto ADS1115::getPga(int ch) const -> CFG_PGA 
{ 
    return fPga[ch]; 
}

void ADS1115::setRate(unsigned int rate) 
{ 
    fRate = rate & 0x07; 
}

auto ADS1115::getRate() const -> unsigned int 
{ 
    return fRate; 
}

void ADS1115::setPga(std::uint8_t channel, CFG_PGA pga)
{
    if (channel > 3) {
        return;
    }
    fPga[channel] = pga;
}

void ADS1115::setActiveChannel(std::uint8_t channel, bool differential_mode)
{
    fSelectedChannel = channel;
    fDiffMode = differential_mode;
}

auto ADS1115::setContinuousSampling(bool cont_sampling) -> bool
{
    fConvMode = (cont_sampling) ? CONV_MODE::CONTINUOUS : CONV_MODE::SINGLE;
    return writeConfig();
}

auto ADS1115::writeConfig(bool startNewConversion) -> bool
{
    std::uint16_t conf_reg { 0 };

    // read in the current contents of config reg only if conv_mode is unknown
    if (fConvMode == CONV_MODE::UNKNOWN) {
        if (read(static_cast<std::uint8_t>(REG::CONFIG), &conf_reg) == 0) {
            return false;
        }
        if ((conf_reg & 0x0100) == 0) {
            fConvMode = CONV_MODE::CONTINUOUS;
        } else {
            fConvMode = CONV_MODE::SINGLE;
        }
    }

    conf_reg = 0;

    if (fConvMode == CONV_MODE::SINGLE && startNewConversion) {
        conf_reg = 0x8000; // set OS bit
    }
    if (!fDiffMode) {
        conf_reg |= 0x4000; // single ended mode channels
    }
    conf_reg |= (fSelectedChannel & 0x03) << 12; // channel select
    if (fConvMode == CONV_MODE::SINGLE) {
        conf_reg |= 0x0100; // single shot mode
    }
    conf_reg |= (static_cast<std::uint8_t>(fPga[fSelectedChannel]) & 0x07) << 9; // PGA gain select

    // This sets the 8 LSBs of the config register (bits 7-0)
    conf_reg |= 0x00; // TODO: enable ALERT/RDY pin
    conf_reg |= (fRate & 0x07) << 5;

    if (write(static_cast<std::uint8_t>(REG::CONFIG), &conf_reg) == 0) {
        return false;
    }
    fCurrentChannel = fSelectedChannel;
    return true;
}

void ADS1115::waitConversionFinished(bool& error)
{
    std::uint16_t conf_reg { 0 };
    // Wait for the conversion to complete, this requires bit 15 to change from 0->1
    int nloops = 0;
    while ((conf_reg & 0x8000) == 0 && nloops * fReadWaitDelay / 1000 < 1000) // readBuf[0] contains 8 MSBs of config register, AND with 10000000 to select bit 15
    {
        std::this_thread::sleep_for(std::chrono::microseconds(fReadWaitDelay));
        if (read(static_cast<std::uint8_t>(REG::CONFIG), &conf_reg) == 0) {
            error = true;
            return;
        }
        nloops++;
    }
    if (nloops * fReadWaitDelay / 1000 >= 1000) {
        error = true;
        return;
    }
    if (nloops > 1) {
        fReadWaitDelay += (nloops - 1) * fReadWaitDelay / 10;
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
    std::lock_guard<std::mutex> lock(fMutex);
    std::int16_t conv_result { 0 }; // Stores the 16 bit value of our ADC conversion

    fConvMode = CONV_MODE::SINGLE;
    fSelectedChannel = channel;

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
        adcToVoltage(conv_result, fPga[fCurrentChannel]),
        lsb_voltage(fPga[fCurrentChannel]),
        fCurrentChannel
    };
    if (fConvReadyFn && sample != InvalidSample) {
        fConvReadyFn(sample);
    }

    if (fAGC[fCurrentChannel]) {
        int eadc = std::abs(conv_result);
        if (eadc > HI_RANGE_LIMIT && fPga[fCurrentChannel] > PGA6V) {
            fPga[fCurrentChannel] = CFG_PGA(fPga[fCurrentChannel] - 1);
            // if (fDebugLevel > 1) printf("ADC input high...setting PGA to level %d\n", fPga[fCurrentChannel]);
        } else if (eadc < LO_RANGE_LIMIT && fPga[fCurrentChannel] < PGA256MV) {
            fPga[fCurrentChannel] = CFG_PGA(fPga[fCurrentChannel] + 1);
            // if (fDebugLevel > 1) printf("ADC input low...setting PGA to level %d\n", fPga[fCurrentChannel]);
        }
    }
    fLastSample[fCurrentChannel] = sample;
    return sample;
}

auto ADS1115::triggerConversion(unsigned int channel) -> bool
{
    // triggering a conversion makes only sense in single shot mode
    if (fConvMode == CONV_MODE::SINGLE) {
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
    std::lock_guard<std::mutex> lock(fMutex);
    std::int16_t conv_result { 0 }; // Stores the 16 bit value of our ADC conversion

    if (!readConversionResult(conv_result)) {
        return InvalidSample;
    }

    stop_timer();
    start_timer();

    Sample sample {
        std::chrono::steady_clock::now(),
        conv_result,
        adcToVoltage(conv_result, fPga[fCurrentChannel]),
        lsb_voltage(fPga[fCurrentChannel]),
        fCurrentChannel
    };
    if (fConvReadyFn && sample != InvalidSample) {
        fConvReadyFn(sample);
    }

    if (fAGC[fCurrentChannel]) {
        int eadc = std::abs(conv_result);
        if (eadc > HI_RANGE_LIMIT && fPga[fCurrentChannel] > PGA6V) {
            fPga[fCurrentChannel] = CFG_PGA(fPga[fCurrentChannel] - 1);
            // if (fDebugLevel > 1) printf("ADC input high...setting PGA to level %d\n", fPga[fCurrentChannel]);
        } else if (eadc < LO_RANGE_LIMIT && fPga[fCurrentChannel] < PGA256MV) {
            fPga[fCurrentChannel] = CFG_PGA(fPga[fCurrentChannel] + 1);
            // if (fDebugLevel > 1) printf("ADC input low...setting PGA to level %d\n", fPga[fCurrentChannel]);
        }
    }
    fLastSample[fCurrentChannel] = sample;
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
    return fReadWaitDelay; 
}

void ADS1115::registerConversionReadyCallback(std::function<void(Sample)> fn) 
{ 
    fConvReadyFn = std::move(fn); 
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
    fAGC[0] = fAGC[1] = fAGC[2] = fAGC[3] = state;
}

void ADS1115::setAGC(std::uint8_t channel, bool state)
{
    if (channel > 3) {
        return;
    }
    fAGC[channel] = state;
}

auto ADS1115::getAGC(std::uint8_t channel) const -> bool
{
    return fAGC[channel & 0x03];
}

} // namespace muonpi::serial::devices
