#include <muonpi/log.h>
#include <muonpi/serial/i2cbus.h>
#include <muonpi/serial/i2cdevices.h>

#include <iostream>
#include <iomanip>
#include <cinttypes>

using namespace muonpi;

auto main() -> int
{
    log::system::setup(muonpi::log::Level::Info, [](int c){exit(c);}, std::cerr);

    serial::i2c_bus bus { "/dev/i2c-1" };
    if ( !bus.general_call.reset() ) {
        log::error() << "resetting bus through general call command";
    }
    
    log::info() << "scanning bus " << bus.address() <<" for devices...";
    
    for ( std::uint8_t addr = 4; addr < 0x7c; ++addr ) {

        auto& dev = bus.open<serial::i2c_device>( addr );

        if ( dev.is_open() && dev.present() ) {
            log::info() << "found " << dev.name() << " at 0x" 
            << std::hex << std::setw(2) << std::setfill('0') 
            << static_cast<int>( addr );
        } else {
            bus.close( addr );
        }
    }

    bool ok { false };
    constexpr std::uint8_t tempsens_addr { 0x4b };
    ok = bus.identify_device<serial::devices::MIC184>( tempsens_addr );
    if (ok) {
        // found the specific device at the expected position, so close the previously created generic
        // i2c_device and reopen as temp sensor device
        bus.close( tempsens_addr );
        auto& tempsensor = bus.open<serial::devices::MIC184>( tempsens_addr );
        log::info()<<"identified MIC184 at 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>( tempsens_addr ) <<" : temp=" << std::dec << tempsensor.get_temperature();
    } else {
        log::error()<<"error identifying MIC184 at 0x" 
        << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>( tempsens_addr );
    }

    constexpr std::uint8_t adc_addr { 0x48 };
    ok = bus.identify_device<serial::devices::ADS1115>( adc_addr );
    if (ok) {
        // found the specific device at the expected position, so close the previously created generic
        // i2c_device and reopen as adc device
        bus.close( adc_addr );
        auto& adc = bus.open<serial::devices::ADS1115>( adc_addr );
        log::info()<<"identified ADS1115 at 0x" 
        << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>( adc_addr ) 
        <<" : ch0=" << std::dec << adc.getVoltage( 0 )
        <<" ch1=" << std::dec << adc.getVoltage( 1 )
        <<" ch2=" << std::dec << adc.getVoltage( 2 )
        <<" ch3=" << std::dec << adc.getVoltage( 3 );
    } else {
        log::error()<<"error identifying ADS1115 at 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>( adc_addr );
    }

    constexpr std::uint8_t i2c_extender_addr { 0x41 };
    ok = bus.identify_device<serial::devices::PCA9536>( i2c_extender_addr );
    if (ok) {
        // found the specific device at the expected position, so close the previously created generic
        // i2c_device and reopen as PCA9536 device
        bus.close( i2c_extender_addr );
        auto& pca = bus.open<serial::devices::PCA9536>( i2c_extender_addr );
        auto input_state { pca.getInputState() };
        if ( !input_state ) {
            log::error() << "reading PCA9536 input state register";
        } else {
            log::info()<<"identified PCA9536 at 0x"
            << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>( i2c_extender_addr ) 
            <<" : inputs=0x" <<std::setw(1) << static_cast<int>(input_state.value()) << std::dec;
        }
    } else {
        log::error()<<"error identifying PCA9536 at 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>( i2c_extender_addr );
    }
    
    log::info()<<"nr of instatiated devices: "<<bus.count_devices();

    // close all devices previously found
    while ( bus.count_devices() != 0U ) {
        auto addr = bus.get_devices().begin()->second->address();
        bus.close( addr );
    }
    log::debug()<<"nr of rx bytes: "<<bus.rx_bytes();
    log::debug()<<"nr of tx bytes: "<<bus.tx_bytes();
}
