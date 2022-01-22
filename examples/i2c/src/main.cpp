#include <muonpi/log.h>
#include <muonpi/serial/i2cbus.h>
#include <muonpi/serial/i2cdevice.h>
#include <muonpi/serial/i2cdevices/lm75.h>

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

		serial::devices::LM75& dev = bus.open<serial::devices::LM75>( addr );

		if ( dev.is_open() && dev.present() ) {
			if ( dev.identify() ) {
				log::info() << "found " << dev.title() << " at 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>( addr );
				log::info() << " temp="<<std::dec<<dev.getTemperature();
			} else {
				log::info() << "found i2c device at 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>( addr );
			}
		} else {
			bus.close( addr );
		}
	}

	// close all devices found
	log::info()<<"nr of instatiated devices: "<<bus.count_devices();
	while ( bus.count_devices() ) {
		auto addr = bus.get_devices().begin()->second->address();
		bus.close( addr );
	}
	log::debug()<<"nr of rx bytes: "<<bus.rx_bytes();
	log::debug()<<"nr of tx bytes: "<<bus.tx_bytes();
}
