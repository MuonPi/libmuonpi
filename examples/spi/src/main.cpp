#include <muonpi/log.h>
#include <muonpi/serial/spidevice.h>

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <array>
#include <chrono>

using namespace muonpi;

auto main() -> int
{
    log::system::setup(muonpi::log::Level::Info, [](int c){exit(c);}, std::cerr);

    log::info() << "spi example ";
    
    serial::spi_device spidev { "/dev/spidev0.1" };
    auto config = spidev.config();
    config.clk_rate = 32'000'000UL;
    config.mode |= static_cast<serial::spi_device::mode_t>(serial::spi_device::MODE::MODE0);
    if ( !spidev.set_config( config ) ) {
        log::error()<<"setting spi configuration";
        exit(1);
    }

    std::array<std::uint8_t, 2> buf { };
    if ( !spidev.read( buf.data(), buf.size()) ) {
        log::error()<<"reading bytes from device "<<spidev.name();
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
    
    std::uint16_t dataword { };
    if ( !spidev.read( &dataword ) ) {
        log::error()<<"reading word from device "<<spidev.name();
    } else {
        log::info()<<"single word read = 0x"<<std::hex<<std::setw(4)<<dataword;
    }

    constexpr unsigned long NLoops { 100000UL };
    auto t_start = std::chrono::system_clock::now();
    for ( auto loop = NLoops; loop; --loop ) {
        std::array<std::uint8_t, 2> sample_buf { };
        if ( !spidev.read( sample_buf.data(), sample_buf.size()) ) {
            log::error()<<"reading bytes from device "<<spidev.name();
        } else {
            unsigned int sample { static_cast<unsigned int>(sample_buf[0]) << 5 };
            sample |= sample_buf[1] >> 2;
            if ( (loop % 10000) == 0 )
                log::info()<<"sample="<<std::dec<<sample;
        }
    }
    auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - t_start);
    
    log::debug()<<"readout time for "<<NLoops<<" r/o loops: "<<dur_ms.count()<<" ms";
    log::debug()<<"nr of rx bytes: "<<spidev.transferred_bytes();
    log::debug()<<"throughput: "<<static_cast<double>(spidev.transferred_bytes())/dur_ms.count()<<"kB/s";
}
