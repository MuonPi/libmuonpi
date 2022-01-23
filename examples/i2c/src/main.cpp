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
	
	log::info() << "scanning bus " << bus.address() <<" for devices...";
	
	for ( std::uint8_t addr = 4; addr < 0x7c; ++addr ) {

		serial::i2c_device& dev = bus.open<serial::i2c_device>( addr );

		if ( dev.is_open() && dev.present() ) {
			log::info() << "found " << dev.title() << " at 0x" 
			<< std::hex << std::setw(2) << std::setfill('0') 
			<< static_cast<int>( addr );
		} else {
			bus.close( addr );
		}
	}

	bool ok { false };
	constexpr uint8_t tempsens_addr { 0x4b };
	ok = bus.identify_device<serial::devices::MIC184>( tempsens_addr );
	if (ok) {
		// found the specific device at the expected position, so close the previously created generic
		// i2c_device and reopen as temp sensor device
		bus.close( tempsens_addr );
		serial::devices::MIC184& tempsensor = bus.open<serial::devices::MIC184>( tempsens_addr );
		log::info()<<"identified MIC184 at 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>( tempsens_addr ) <<" : temp=" << std::dec << tempsensor.get_temperature();
	} else {
		log::error()<<"error identifying MIC184 at 0x" 
		<< std::hex << std::setw(2) << std::setfill('0') << static_cast<int>( tempsens_addr );
	}

	constexpr uint8_t adc_addr { 0x48 };
	ok = bus.identify_device<serial::devices::ADS1115>( 0x48 );
	if (ok) {
		// found the specific device at the expected position, so close the previously created generic
		// i2c_device and reopen as adc device
		bus.close( adc_addr );
		serial::devices::ADS1115& adc = bus.open<serial::devices::ADS1115>( adc_addr );
		log::info()<<"identified ADS1115 at 0x" 
		<< std::hex << std::setw(2) << std::setfill('0') << static_cast<int>( adc_addr ) 
		<<" : ch0=" << std::dec << adc.getVoltage( 0 )
		<<" ch1=" << std::dec << adc.getVoltage( 1 )
		<<" ch2=" << std::dec << adc.getVoltage( 2 )
		<<" ch3=" << std::dec << adc.getVoltage( 3 );
	} else {
		log::error()<<"error identifying ADS1115 at 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>( adc_addr );
	}

	log::info()<<"nr of instatiated devices: "<<bus.count_devices();

	// close all devices previously found
	while ( bus.count_devices() ) {
		auto addr = bus.get_devices().begin()->second->address();
		bus.close( addr );
	}
	log::debug()<<"nr of rx bytes: "<<bus.rx_bytes();
	log::debug()<<"nr of tx bytes: "<<bus.tx_bytes();
}
