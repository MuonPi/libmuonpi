#include "muonpi/serial/i2cdevices/eeprom24aa02.h"
#include <stdint.h>
#include <unistd.h>
#include <thread>
#include <chrono>

namespace muonpi::serial::devices {

/*
* 24AA02 EEPROM
*/

constexpr std::chrono::microseconds EEP_WRITE_IDLE_TIME { 5000 };
constexpr std::size_t PAGESIZE { 8 };

EEPROM24AA02::EEPROM24AA02(i2c_bus& bus, std::uint8_t address)
	: i2c_device( bus, address )
{
	set_title("24AA02");
}

auto EEPROM24AA02::read(std::uint8_t start_addr, std::uint8_t* buffer, std::size_t bytes ) -> int
{
	start_timer();
	int nbytes = i2c_device::read( start_addr, buffer, bytes );
	stop_timer();
	return nbytes;
}

auto EEPROM24AA02::read_byte( std::uint8_t addr, std::uint8_t* value ) -> bool
{
	return ( this->read( addr, value, static_cast<std::size_t>(1) ) == 1 );
}

auto EEPROM24AA02::writeByte(uint8_t addr, uint8_t data) -> bool
{
    uint8_t writeBuf[2] { addr, data }; // Buffer to store the 2 bytes that we write to the I2C device

    start_timer();

    // Write address and data byte
    if ( write(writeBuf, 2) != 2 ) return false;
	
	// wait for eep to finish the writing process
	// TODO: move the write to another thread to continue code execution in the calling thread
	// but block any access to the eeprom during write cycle
	std::this_thread::sleep_for( EEP_WRITE_IDLE_TIME );
    stop_timer();
	return true;
}

auto EEPROM24AA02::write(std::uint8_t addr, std::uint8_t* buffer, std::size_t bytes ) -> int
{
    if ( bytes == 1 ) {
		if ( writeByte( addr, *buffer ) ) {
			return 1;
		} else {
			return 0;
		}
	}
    int total_written { 0 };
    start_timer();
    for (uint16_t i = 0; i < bytes;) {
        uint8_t currAddr = addr + i;
        // determine, how many bytes left on current page
        uint8_t pageRemainder = PAGESIZE - currAddr % PAGESIZE;
        if (currAddr + pageRemainder >= bytes)
            pageRemainder = bytes - currAddr;
        int n = i2c_device::write( currAddr, &buffer[i], pageRemainder );
        std::this_thread::sleep_for( EEP_WRITE_IDLE_TIME );
        i += pageRemainder;
		total_written += n;
    }
    stop_timer();
    return total_written;
}

auto EEPROM24AA02::identify() -> bool
{
	if ( flag_set(Flags::Failed) ) return false;
	if ( !present() ) return false;

	const unsigned int N { 256 };
	uint8_t buf[N+1];
//	std::cout << " attempt 1: offs=0, len="<<N<<std::endl;
	if ( read( 0x00, buf, N ) != N) {
		// somehow did not read exact same amount of bytes as it should
		return false;
	}
//	std::cout << " attempt 2: offs=1, len="<<N<<std::endl;
	if ( read( 0x01, buf, N ) != N) {
		// somehow did not read exact same amount of bytes as it should
		return false;
	}
//	std::cout << " attempt 3: offs=0, len="<<N+1<<std::endl;
	if ( read( 0x00, buf, N+1 ) != N+1) {
		// somehow did not read exact same amount of bytes as it should
		return false;
	}
//	std::cout << " attempt 4: offs=0xfa, len="<<int(6)<<std::endl;
	if ( read( 0xfa, buf, 6 ) != 6) {
		// somehow did not read exact same amount of bytes as it should
		return false;
	}

	// seems, we have a 24AA02 (or larger) at this point
	// additionaly check, whether it could be a 24AA02UID, 
	// i.e. if the last 6 bytes contain 2 bytes of vendor/device code and 4 bytes of unique id
	if ( buf[0] == 0x29 && buf[1] == 0x41 ) {
		set_title("24AA02UID");
	}
	return true;
}

} // namespace muonpi::serial::devices