#ifndef MUONPI_SERIAL_I2CDEVICES_GENERIC_EEPROM_H
#define MUONPI_SERIAL_I2CDEVICES_GENERIC_EEPROM_H

#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"

#include <cstring>
#include <sstream>
#include <type_traits>

namespace muonpi::serial::devices {

/**
 * @brief The i2c_eeprom class.
 * Generic class template for i2c eeprom devices. The template parameters (and thus the properties
 * assumed to vary between different device types) are: <p><ul> <li>EEPLENGTH: The total capacity in
 * bytes of the eeprom <li>ADDRESSMODE: whether the device addressing is single-byte (ADDRESSMODE=1)
 * or allows word addresses (ADDRESSMODE=2) <li>PAGELENGTH: The page size in bytes which can be
 * written in one chunk
 * </ul><p>
 * @note The class overwrites the @link i2c_device i2c_device @endlink base class' methods @link
 * i2c_device#read read @endlink and
 * @link i2c_device#write write @endlink in order to manage the paged access correctly. This is
 * intended behavior.
 * @note Devices with ADDRESSMODE=1 may occupy more than one i2c address on the bus. For this end,
 * an additional member base_address keeps track of the primary address while the @link
 * i2c_device#address address @endlink property may change during read/write operations.
 * @note This class provides two templated versions for either @link #read read @endlink and @link
 * #write write @endlink methods for 8bit register and 16bit register access, respectively.
 * Depending on the address mode (ADDRESSMODE template param), only one of the two methods
 * participates in overload resolution. In this way, there are only 16bit versions of read and write
 * visible for devices instantiated as two-byte register access eeproms (address mode 2) for
 * instance.
 */
template <std::size_t EEPLENGTH = 256, std::uint8_t ADDRESSMODE = 1, std::size_t PAGELENGTH = 8>
class i2c_eeprom : public i2c_device, public static_device_base<i2c_eeprom<EEPLENGTH,ADDRESSMODE,PAGELENGTH>> {
public:
    explicit i2c_eeprom(i2c_bus& bus, std::uint8_t base_address = InvalidI2cAddress);

    /** Read multiple bytes starting from given address from EEPROM memory.
     * @param start_addr First 16bit register address to read from
     * @param buffer Buffer to receive the data read from eeprom
     * @param num_bytes Number of bytes to write
     * @return Number of bytes actually read
     * @note this is an overriding function to the one in the i2c_device base class in order to
     * handle the i2c subaddress management correctly for EEPROMs with address mode 2.
     */
    template <std::uint8_t AM = ADDRESSMODE>
    [[nodiscard]] typename std::enable_if_t<AM == 2, int>
    read(std::uint16_t start_addr, std::uint8_t* buffer, std::size_t num_bytes = 1);

    /** Read multiple bytes starting from given address from EEPROM memory.
     * @param start_addr First 8bit register address to read from
     * @param buffer Buffer to receive the data read from eeprom
     * @param num_bytes Number of bytes to write
     * @return Number of bytes actually read
     * @note this is an overriding function to the one in the i2c_device base class in order to
     * handle the i2c subaddress management correctly for EEPROMs with address mode 1. For those,
     * the hi-byte of the address is encrypted as access to an i2c address different from the
     * device's base address. example: a 512 byte device may provide the data through two i2c
     * addresses 0x50 (the base address) and 0x51. Reading and writing data to these two addresses
     * accesses the two 256-byte pages the memory is subdivided into.
     */
    template <std::uint8_t AM = ADDRESSMODE>
    [[nodiscard]] typename std::enable_if_t<AM == 1, int>
    read(std::uint8_t start_addr, std::uint8_t* buffer, std::size_t num_bytes = 1);

    /** Write multiple bytes starting from given address into EEPROM memory.
     * @param addr First 16bit register address to write to
     * @param buffer Buffer to copy new data from
     * @param num_bytes Number of bytes to write
     * @return Number of bytes actually written
     * @note this is an overriding function to the one in the i2c_device base class in order to
     * prevent sequential write operations crossing page boundaries of the EEPROM. This function
     * conforms to the page-wise sequential write (c.f.
     * http://ww1.microchip.com/downloads/en/devicedoc/21709c.pdf  p.7).
     */
    template <std::uint8_t AM = ADDRESSMODE>
    [[nodiscard]] typename std::enable_if_t<AM == 2, int>
    write(std::uint16_t addr, std::uint8_t* buffer, std::size_t num_bytes = 1);
    // TODO: method in base class is not virtual. Nor is this method marked override.
    // Reply: the method shall explicitely shadow the base class' method to never be used for memory
    // access to the eeprom directly

    /** Write multiple bytes starting from given address into EEPROM memory.
     * @param addr First 8bit register address to write to
     * @param buffer Buffer to copy new data from
     * @param num_bytes Number of bytes to write
     * @return Number of bytes actually written
     * @note this is an overriding function to the one in the i2c_device base class in order to
     * prevent sequential write operations crossing page boundaries of the EEPROM. This function
     * conforms to the page-wise sequential write (c.f.
     * http://ww1.microchip.com/downloads/en/devicedoc/21709c.pdf  p.7).
     */
    template <std::uint8_t AM = ADDRESSMODE>
    [[nodiscard]] typename std::enable_if_t<AM == 1, int>
    write(std::uint8_t addr, std::uint8_t* buffer, std::size_t num_bytes = 1);

    [[nodiscard]] auto identify() -> bool override;

    /** the total capacity of the eeprom memory
     * @return eeprom memory size in bytes
     */
    [[nodiscard]] static constexpr auto size() -> std::size_t {
        return EEPLENGTH;
    }

    /** the page size of the eeprom
     * @return number of bytes per page
     */
    [[nodiscard]] static constexpr auto page_size() -> std::size_t {
        return PAGELENGTH;
    }

    /** the address mode of the eeprom
     * <p> The address mode describes whether the access to memory locations is managed through 8bit
     * or 16bit addresses
     * @return the address mode: 1=single byte addressing, 2=word addressing
     */
    [[nodiscard]] static constexpr auto address_mode() -> std::uint8_t {
        return ADDRESSMODE;
    }

    /** get the i2c base address of the device
     * @return i2c base address which is occupied by the device
     * @note eeproms with single-byte addressing (ADDRESSMODE=1) and capacities beyond a full
     * 8bit-addressable range encode the hi-byte of the memory address as access to an i2c device
     * address incremental to the base address. In this way, different 256-Byte pages are accessed
     * through consecutive i2c device addresses. The first page is accessed through base_address,
     * the second through (base_address+1) and so forth. Accesses through
     * @link #read read @endlink and @link #write write @endlink methods automatically
     * take care for address switching.
     */
    [[nodiscard]] auto base_address() const -> std::uint8_t {
        return m_base_address;
    }

private:
    std::uint8_t m_base_address {0xff}; ///<! the base address of the device on the bus
    static constexpr std::chrono::microseconds EEP_WRITE_IDLE_TIME {5000};
    static constexpr std::size_t               MAX_READ_BLOCK_SIZE {256};
};

/*********************
 * Implementation part
 *********************/
template <std::size_t EEPLENGTH, std::uint8_t ADDRESSMODE, std::size_t PAGELENGTH>
i2c_eeprom<EEPLENGTH, ADDRESSMODE, PAGELENGTH>::i2c_eeprom(i2c_bus& bus, std::uint8_t base_address)
    : i2c_device(bus, base_address)
    , m_base_address(base_address) {
    static_assert(ADDRESSMODE == 1 || ADDRESSMODE == 2,
                  "unknown address mode for eeprom (must be 1 or 2)");
    static_assert(EEPLENGTH <= 65536UL, "unsupported eeprom size (must be <=65536)");
    set_name("EEPROM");
    set_addresses_hint({0x50});
}

template <std::size_t EEPLENGTH, std::uint8_t ADDRESSMODE, std::size_t PAGELENGTH>
template <std::uint8_t AM>
typename std::enable_if_t<AM == 2, int>
i2c_eeprom<EEPLENGTH, ADDRESSMODE, PAGELENGTH>::write(std::uint16_t addr,
                                                      std::uint8_t* buffer,
                                                      std::size_t   num_bytes) {
    int total_written {0};

    scope_guard timer_guard {setup_timer()};

    for (std::size_t i = 0; i < num_bytes;) {
        std::size_t currAddr {addr + i};
        // determine, how many bytes left on current page
        std::size_t pageRemainder = PAGELENGTH - currAddr % PAGELENGTH;
        if (pageRemainder > num_bytes - total_written) {
            pageRemainder = num_bytes - total_written;
        }

        auto write_buffer = std::make_unique<std::uint8_t[]>(pageRemainder + 2u);

        write_buffer[0] = static_cast<std::uint8_t>((currAddr >> 8) & 0xff);
        write_buffer[1] = static_cast<std::uint8_t>(currAddr & 0xff);

        std::memcpy(write_buffer.get() + 2, &buffer[i], pageRemainder);

        // write data block
        const int n {write(write_buffer.get(), pageRemainder + 2) - 2};

        if (n < 0)
            return total_written;
        std::this_thread::sleep_for(EEP_WRITE_IDLE_TIME);
        i += n;
        total_written += n;
    }
    return total_written;
}

template <std::size_t EEPLENGTH, std::uint8_t ADDRESSMODE, std::size_t PAGELENGTH>
template <std::uint8_t AM>
typename std::enable_if_t<AM == 1, int>
i2c_eeprom<EEPLENGTH, ADDRESSMODE, PAGELENGTH>::write(std::uint8_t  addr,
                                                      std::uint8_t* buffer,
                                                      std::size_t   num_bytes) {
    int total_written {0};

    scope_guard timer_guard {setup_timer()};

    for (std::size_t i = 0; i < num_bytes;) {
        std::size_t currAddr {addr + i};
        // determine, how many bytes left on current page
        std::size_t pageRemainder = PAGELENGTH - currAddr % PAGELENGTH;
        if (pageRemainder > num_bytes - total_written) {
            pageRemainder = num_bytes - total_written;
        }

        if (address() != m_base_address + (currAddr >> 8)) {
            set_address(m_base_address + (currAddr >> 8));
        }
        const int n {i2c_device::write(static_cast<std::uint8_t>(currAddr & 0xff),
                                       &buffer[i],
                                       pageRemainder)};

        if (n < 0)
            return total_written;
        std::this_thread::sleep_for(EEP_WRITE_IDLE_TIME);
        i += n;
        total_written += n;
    }
    return total_written;
}

template <std::size_t EEPLENGTH, std::uint8_t ADDRESSMODE, std::size_t PAGELENGTH>
template <std::uint8_t AM>
typename std::enable_if_t<AM == 2, int>
i2c_eeprom<EEPLENGTH, ADDRESSMODE, PAGELENGTH>::read(std::uint16_t start_addr,
                                                     std::uint8_t* buffer,
                                                     std::size_t   num_bytes) {
    int total_read {0};

    scope_guard timer_guard {setup_timer()};

    for (std::size_t i = 0; i < num_bytes;) {
        std::size_t currAddr {start_addr + i};
        // determine, how many bytes left on current page
        std::size_t pageRemainder {MAX_READ_BLOCK_SIZE - currAddr % MAX_READ_BLOCK_SIZE};
        if (pageRemainder > num_bytes - total_read) {
            pageRemainder = num_bytes - total_read;
        }

        // write 16bit address first
        std::array<std::uint8_t, 2> addr_bytes {static_cast<std::uint8_t>((currAddr >> 8) & 0xff),
                                                static_cast<std::uint8_t>(currAddr & 0xff)};
        if (i2c_device::write(addr_bytes.data(), 2u) != 2) {
            return total_read;
        }
        // read data block
        const int n {i2c_device::read(&buffer[i], pageRemainder)};

        if (n < 0)
            return total_read;
        i += n;
        total_read += n;
    }
    return total_read;
}

template <std::size_t EEPLENGTH, std::uint8_t ADDRESSMODE, std::size_t PAGELENGTH>
template <std::uint8_t AM>
typename std::enable_if_t<AM == 1, int>
i2c_eeprom<EEPLENGTH, ADDRESSMODE, PAGELENGTH>::read(std::uint8_t  start_addr,
                                                     std::uint8_t* buffer,
                                                     std::size_t   num_bytes) {
    int total_read {0};

    scope_guard timer_guard {setup_timer()};

    for (std::size_t i = 0; i < num_bytes;) {
        std::size_t currAddr {start_addr + i};
        // determine, how many bytes left on current page
        std::size_t pageRemainder {MAX_READ_BLOCK_SIZE - currAddr % MAX_READ_BLOCK_SIZE};
        if (pageRemainder > num_bytes - total_read) {
            pageRemainder = num_bytes - total_read;
        }

        if (address() != m_base_address + (currAddr >> 8)) {
            set_address(m_base_address + (currAddr >> 8));
        }
        const int n {i2c_device::read(static_cast<std::uint8_t>(currAddr & 0xff),
                                      &buffer[i],
                                      pageRemainder)};

        if (n < 0)
            return total_read;
        i += n;
        total_read += n;
    }
    return total_read;
}

template <std::size_t EEPLENGTH, std::uint8_t ADDRESSMODE, std::size_t PAGELENGTH>
auto i2c_eeprom<EEPLENGTH, ADDRESSMODE, PAGELENGTH>::identify() -> bool {
    if (flag_set(Flags::Failed) || !present()) {
        return false;
    }

    // identifying the device:
    // read a data block with the size equal to the capacity of the eeprom in one chunk.
    // if the number of bytes actually read is as expected, we may assume that we really have en
    // eeprom with at least the specified size
    std::array<std::uint8_t, EEPLENGTH> buf {};
    if (read(0, buf.data(), EEPLENGTH) != EEPLENGTH) {
        // somehow did not read exact same amount of bytes as it should
        return false;
    }

    if (EEPLENGTH == 256UL && ADDRESSMODE == 1) {
        if (read(0xfa, buf.data(), 6u) != 6) {
            // somehow did not read exact same amount of bytes as it should
            return false;
        }
        // seems, we have a 24AA02 at this point
        // additionaly check, whether it could be a 24AA02UID,
        // i.e. if the last 6 bytes contain 2 bytes of vendor/device code and 4 bytes of unique id
        if (buf[0] == 0x29u && buf[1] == 0x41u) {
            set_name(name() + " 24AA02UID");
        }
    } else {
        std::ostringstream ostr;
        ostr << ' ' << EEPLENGTH << 'B';
        set_name(name() + ostr.str());
    }
    return true;
}

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_GENERIC_EEPROM_H
