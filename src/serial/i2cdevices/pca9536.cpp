#include "muonpi/serial/i2cdevices/pca9536.h"
#include <stdint.h>
#include <iostream>
#include <iomanip>

namespace muonpi::serial::devices {

/*
* PCA9536 4 pin I/O expander
*/

PCA9536::PCA9536(i2c_bus& bus, std::uint8_t address)
	: i2c_device( bus, address )
{
	set_title("PCA9536");
}

bool PCA9536::setOutputPorts( std::uint8_t portMask)
{
    std::uint8_t data = ~portMask;
    start_timer();
    if ( 1 != write( REG::CONFIG, &data, 1 ) ) {
        return false;
    }
    stop_timer();
    return true;
}

bool PCA9536::setOutputState( std::uint8_t portMask )
{
    start_timer();
    if ( 1 != write( REG::OUTPUT, &portMask, 1 ) ) {
        return false;
    }
    stop_timer();
    return true;
}

auto PCA9536::getInputState( std::uint8_t* state ) -> bool
{
    std::uint8_t inport = 0x00;
    start_timer();
    if ( 1 != read(REG::INPUT, &inport, 1) ) return false;
    stop_timer();
    *state = inport & 0x0f;
	return true;
}

auto PCA9536::present() -> bool
{
    std::uint8_t inport = 0x00;
    // read input port
    return ( 1 == read( REG::INPUT, &inport, 1 ) );
}

auto PCA9536::identify() -> bool
{
	if ( flag_set(Flags::Failed) ) return false;
	if ( !present() ) return false;

	std::uint8_t bytereg { 0 };
/*	
	for (int i=0; i<256; i++) {
		if ( !readByte( static_cast<uint8_t>(i), &bytereg ) ) break;
		std::cout << "reg 0x"<<std::hex<<std::setfill('0')<<std::setw(2)<<i<<" : 0x"<<(int)bytereg<<std::endl;
	}
*/	
	if ( !read( static_cast<std::uint8_t>(REG::INPUT), &bytereg ) ) {
		// there was an error
		return false;
	}
	if ( ( bytereg & 0xf0 ) != 0xf0 ) return false;
/*
	if ( !readByte( static_cast<uint8_t>(REG::OUTPUT), &bytereg ) ) {
		// there was an error
		return false;
	}
	if ( ( bytereg & 0xf0 ) != 0xf0 ) return false;
*/
	if ( !read( static_cast<std::uint8_t>(REG::POLARITY), &bytereg ) ) {
		// there was an error
		return false;
	}
	if ( ( bytereg & 0xf0 ) != 0x00 ) return false;

	if ( !read( static_cast<std::uint8_t>(REG::CONFIG), &bytereg ) ) {
		// there was an error
		return false;
	}
	if ( ( bytereg & 0xf0 ) != 0xf0 ) return false;
	if ( read( static_cast<std::uint8_t>(0x04), &bytereg, 1) > 0 ) return false;
	return true;
}

} // namespace muonpi::serial::devices