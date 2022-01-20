#include <muonpi/log.h>
#include <muonpi/serial/i2cbus.h>
#include <muonpi/serial/i2cdevice.h>

#include <iostream>
#include <iomanip>
#include <cinttypes>

using namespace muonpi;

auto main() -> int
{
	log::system::setup(muonpi::log::Level::Info, [](int c){exit(c);}, std::cerr);

	serial::i2c_bus bus { "/dev/i2c-1" };
	
	for ( std::uint8_t addr = 4; addr < 0x7c; ++addr ) {

		// TODO: The following line should show the correct usage of the i2c_bus::open() function but IT DOESN'T WORK
		// it fails with compilation error saying that the element from reference_wrapper in the i2c_bus::m_devices map
		// cannot by dynamic_cast converted to a reference of the same type, i.e. T& = dynamic_cast<m_devices[addr]> fails
		//		serial::i2c_device& dev = bus.open<serial::i2c_device>( addr );

		// alternatively, an i2c_device object can be created separately
		// but this is clearly not intended to be used in this manner
		std::size_t rx_counter { 0 }, tx_counter { 0 };
		serial::i2c_device dev { bus, rx_counter, tx_counter, addr };
		if ( dev.is_open() && dev.present() ) {
			log::info() << "found i2c device at 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>( addr );
		}
	}
}
