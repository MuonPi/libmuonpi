#ifndef MUONPI_SERIAL_I2CDEVICES_GENERIC_EEPROM_H
#define MUONPI_SERIAL_I2CDEVICES_GENERIC_EEPROM_H

#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"

namespace muonpi::serial::devices {

template <std::size_t EEPLENGTH = 256, std::uint8_t ADDRESSMODE = 1, std::size_t PAGELENGTH = 8>
class i2c_eeprom : public i2c_device {
public:
    explicit i2c_eeprom(i2c_bus& bus, std::uint8_t base_address);

    [[nodiscard]] auto read(std::uint16_t start_addr, std::uint8_t* buffer, std::size_t num_bytes = 1) -> int;
    /** Write multiple bytes starting from given address into EEPROM memory.
     * @param addr First register address to write to
     * @param buffer Buffer to copy new data from
     * @param bytes Number of bytes to write
     * @return Number of bytes actually written
     * @note this is an overriding function to the one in the i2c_device base class in order to
     * prevent sequential write operations crossing page boundaries of the EEPROM. This function conforms to
     * the page-wise sequential write (c.f. http://ww1.microchip.com/downloads/en/devicedoc/21709c.pdf  p.7).
     */
    auto write(std::uint16_t addr, std::uint8_t* buffer, std::size_t num_bytes = 1) -> int; // TODO: method in base class is not virtual. Nor is this method marked override.

    [[nodiscard]] auto identify() -> bool override;
    [[nodiscard]] static constexpr auto size() -> std::size_t { return EEPLENGTH; }
    [[nodiscard]] static constexpr auto page_size() -> std::size_t { return PAGELENGTH; }
    [[nodiscard]] static constexpr auto address_mode() -> std::uint8_t { return ADDRESSMODE; }

private:
    // hide all low level read/write functions from the i2c_device base class since they do not conform
    // to the correct write sequence required by the eeprom and would lead to data corruption when used.
    // they are replaced with methods in the public interface of this class with equal signature
    using i2c_device::read;
    using i2c_device::write;
    std::uint8_t m_base_address { 0xff };
    static constexpr std::chrono::microseconds EEP_WRITE_IDLE_TIME { 5000 };
    static constexpr std::size_t MAX_READ_BLOCK_SIZE { 256 };
};

/*********************
 * Implementation part
 *********************/
template <std::size_t EEPLENGTH, std::uint8_t ADDRESSMODE, std::size_t PAGELENGTH>
i2c_eeprom<EEPLENGTH,ADDRESSMODE,PAGELENGTH>::i2c_eeprom(i2c_bus& bus, std::uint8_t base_address)
    : i2c_device(bus, base_address), m_base_address( base_address )
{
    set_name("EEPROM");
    m_addresses_hint = { 0x50 };
}

template <std::size_t EEPLENGTH, std::uint8_t ADDRESSMODE, std::size_t PAGELENGTH>
auto i2c_eeprom<EEPLENGTH,ADDRESSMODE,PAGELENGTH>::write(std::uint16_t addr, std::uint8_t* buffer, std::size_t num_bytes) -> int
{
    int total_written { 0 };

    scope_guard timer_guard { setup_timer() };

    for (std::size_t i = 0; i < num_bytes;) {
        std::size_t currAddr { addr + i };
        // determine, how many bytes left on current page
        std::size_t pageRemainder = PAGELENGTH - currAddr % PAGELENGTH;
        if ( pageRemainder > num_bytes - total_written ) {
            pageRemainder = num_bytes - total_written;
        }
        
        int n { 0 };
        switch ( ADDRESSMODE ) {
            case 1:
                if ( address() != m_base_address + (currAddr >> 8) ) {
                    set_address( m_base_address + (currAddr >> 8) );
                }
                n = i2c_device::write( static_cast<std::uint8_t>(currAddr & 0xff), &buffer[i], pageRemainder );
                break;
            case 2:
                n = i2c_device::write( static_cast<std::uint16_t>(currAddr), &buffer[i], pageRemainder );
                break;
            default:
                n = -1;
        }
        
        if ( n < 0 ) return total_written;
        std::this_thread::sleep_for(EEP_WRITE_IDLE_TIME);
        i += n;
        total_written += n;
    }
    return total_written;
}

template <std::size_t EEPLENGTH, std::uint8_t ADDRESSMODE, std::size_t PAGELENGTH>
auto i2c_eeprom<EEPLENGTH,ADDRESSMODE,PAGELENGTH>::read(std::uint16_t start_addr, std::uint8_t* buffer, std::size_t num_bytes) -> int
{
    int total_read { 0 };

    scope_guard timer_guard { setup_timer() };

    for (std::size_t i = 0; i < num_bytes;) {
        std::size_t currAddr { start_addr + i };
        // determine, how many bytes left on current page
        std::size_t pageRemainder { MAX_READ_BLOCK_SIZE - currAddr % MAX_READ_BLOCK_SIZE };
        if ( pageRemainder > num_bytes - total_read ) {
            pageRemainder = num_bytes - total_read;
        }
        
        int n { 0 };
        switch ( ADDRESSMODE ) {
            case 1:
                if ( address() != m_base_address + (currAddr >> 8) ) {
                    set_address( m_base_address + (currAddr >> 8) );
                    std::cout<<"DEBUG: resetting i2c address to 0x"<<std::hex<<static_cast<int>(address())<<"\n";
                }
                n = i2c_device::read( static_cast<std::uint8_t>(currAddr & 0xff), &buffer[i], pageRemainder );
                break;
            case 2:
                n = i2c_device::read( static_cast<std::uint16_t>(currAddr), &buffer[i], pageRemainder );
                break;
            default:
                n = -1;
        }
        
        if ( n < 0 ) return total_read;
        i += n;
        total_read += n;
    }
    return total_read;
}

template <std::size_t EEPLENGTH, std::uint8_t ADDRESSMODE, std::size_t PAGELENGTH>
auto i2c_eeprom<EEPLENGTH,ADDRESSMODE,PAGELENGTH>::identify() -> bool
{
    if (flag_set(Flags::Failed)) {
        return false;
    }
    if (!present()) {
        return false;
    }

    constexpr unsigned int N { size() };
    std::array<std::uint8_t, N> buf {};
    if ( read( static_cast<std::uint16_t>(0x0000), buf.data(), N) != N ) {
        // somehow did not read exact same amount of bytes as it should
        return false;
    }

    if ( size() == 256UL && address_mode() == 1 ) {
        if (read( static_cast<std::uint8_t>(0xfa), buf.data(), 6u) != 6) {
            // somehow did not read exact same amount of bytes as it should
            return false;
        }
        // seems, we have a 24AA02 (or larger) at this point
        // additionaly check, whether it could be a 24AA02UID,
        // i.e. if the last 6 bytes contain 2 bytes of vendor/device code and 4 bytes of unique id
        if (buf[0] == 0x29u && buf[1] == 0x41u) {
            set_name(name()+" 24AA02UID");
        }
    }

    return true;
}


} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_GENERIC_EEPROM_H
