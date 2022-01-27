#ifndef MUONPI_SERIAL_I2CDEVICES_EEPROM24AA02_H
#define MUONPI_SERIAL_I2CDEVICES_EEPROM24AA02_H

#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"

namespace muonpi::serial::devices {

/* EEPROM24AA02  */

class EEPROM24AA02 : public i2c_device {
public:
    explicit EEPROM24AA02(i2c_bus& bus, std::uint8_t address);

    [[nodiscard]] auto read(std::uint8_t start_addr, std::uint8_t* buffer, std::size_t bytes = 1) -> int;
    [[nodiscard]] auto read_byte(std::uint8_t addr, std::uint8_t* value) -> bool;

    [[nodiscard]] auto writeByte(uint8_t addr, uint8_t data) -> bool;
    /** Write multiple bytes starting from given address into EEPROM memory.
     * @param addr First register address to write to
     * @param buffer Buffer to copy new data from
     * @param bytes Number of bytes to write
     * @return Number of bytes actually written
     * @note this is an overriding function to the one in the i2c_device base class in order to
     * prevent sequential write operations crossing page boundaries of the EEPROM. This function conforms to
     * the page-wise sequential write (c.f. http://ww1.microchip.com/downloads/en/devicedoc/21709c.pdf  p.7).
     */
    auto write(std::uint8_t addr, std::uint8_t* buffer, std::size_t bytes = 1) -> int; // TODO: method in base class is not virtual. Nor is this method marked override.

    [[nodiscard]] auto identify() -> bool override;

private:
    // hide all low level read/write functions from i2c_device class since they do not conform
    // to the correct write sequence of the eeprom
    // they will be replaced with methods in the public interface with equal signature
    using i2c_device::read;
    using i2c_device::write;
};

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_EEPROM24AA02_H
