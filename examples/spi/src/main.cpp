#include <muonpi/log.h>
#include <muonpi/serial/spidevice.h>

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <array>

using namespace muonpi;

auto main() -> int
{
    log::system::setup(muonpi::log::Level::Info, [](int c){exit(c);}, std::cerr);

    log::info() << "spi example ";
    
    serial::spi_device spidev { "/dev/spidev0.1" };
    auto config = spidev.config();
    config.clk_rate = 1'000'000UL;
    if ( !spidev.set_config( config ) ) {
        throw "error setting spi config";
    }

    std::array<std::uint8_t, 2> buf { };
    if ( !spidev.read( buf.data(), buf.size()) ) {
        log::error()<<"reading from device "<<spidev.name();
    } else {
        std::ostringstream ostr;
        ostr << "bytes read: ";
        for ( auto byte: buf ) {
            ostr<<"0x"<<std::hex<<std::setw(2)<<std::setfill('0')<<static_cast<int>(byte)<<' ';
        }
        log::info()<<ostr.str();
        if ( buf.size() > 1 ) {
            unsigned int sample { static_cast<unsigned int>(buf[0]) << 5 };
            sample |= buf[1] >> 2;
            log::info()<<"sample="<<std::dec<<sample;
        }
    }

    // close all devices previously found
    log::debug()<<"nr of rx bytes: "<<spidev.rx_bytes();
    log::debug()<<"nr of tx bytes: "<<spidev.tx_bytes();
}
