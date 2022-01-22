#include "muonpi/serial/i2cdevices/lm75.h"
#include <stdint.h>
#include <iostream>
#include <iomanip>


namespace muonpi::serial::devices {

/*
* LM75 Temperature Sensor
*/

LM75::LM75(i2c_bus& bus, std::uint8_t address)
	: i2c_device( bus, address )
{
	set_title("LM75");
}

LM75::~LM75()
{
}

int16_t LM75::readRaw()
{
    start_timer();

    uint16_t dataword { 0 };
	// Read the temp register
	if ( !read( static_cast<uint8_t>(REG::TEMP), &dataword ) ) {
		// there was an error
		return INT16_MIN;
	}

	int16_t val = static_cast<int16_t>( dataword );
//    val = ((int16_t)readBuf[0] << 8) | readBuf[1];
    //fLastRawValue = val;

    stop_timer();

    return val;
}

float LM75::getTemperature()
{
	int16_t dataword = readRaw();
	float temp = static_cast<float>( dataword >> 8 );
	temp += static_cast<float>(dataword & 0xff)/256.; 
	//fLastTemp = temp;
    return temp;
}

bool LM75::identify()
{
	if ( flag_set(Flags::Failed) ) return false;
	if ( !present() ) return false;
    uint16_t dataword { 0 };
	uint8_t conf_reg { 0 };
	// Read the config register
	if ( !read( static_cast<uint8_t>(REG::CONF), &conf_reg ) ) {
		// there was an error
		return false;
	}
	// datasheet: 3 MSBs of conf register "should be kept as zeroes"
	if ( ( conf_reg >> 5 ) != 0 ) return false;
	
	// read temp register
	if ( !read( static_cast<uint8_t>(REG::TEMP), &dataword ) ) {
		// there was an error
		return false;
	}
	// the 5 LSBs should always read zero
	if ( (dataword & 0x1f) != 0 ) return false;
//	if ( ( (dataword & 0x1f) != 0 ) && ( dataword >> 5 ) == 0 ) return false;
	
	// read Thyst register
	if ( !read( static_cast<uint8_t>(REG::THYST), &dataword ) ) {
		// there was an error
		return false;
	}
	// the 7 MSBs should always read zero
	if ( (dataword & 0x7f) != 0 ) return false;

	// read Tos register
	if ( !read( static_cast<uint8_t>(REG::TOS), &dataword ) ) {
		// there was an error
		return false;
	}
	// the 7 MSBs should always read zero
	if ( (dataword & 0x7f) != 0 ) return false;
	
	return true;
}

} // namespace muonpi::serial::devices
