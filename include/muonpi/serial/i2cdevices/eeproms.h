#ifndef MUONPI_SERIAL_I2CDEVICES_EEPROMS_H
#define MUONPI_SERIAL_I2CDEVICES_EEPROMS_H
#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevices/generic_eeprom.h"

namespace muonpi::serial::devices::eeproms {

/* the following list of devices and the coresponding properties (size, address mode and page
 * length) is taken and adapted from https://github.com/CombiesGit/I2C_EEPROM
 */

// Atmel
typedef i2c_eeprom<128UL, 1, 8> AT24C01;
typedef i2c_eeprom<256UL, 1, 8> AT24C02;
typedef i2c_eeprom<512UL, 1, 16> AT24C04;
typedef i2c_eeprom<1024UL, 1, 16> AT24C08;
typedef i2c_eeprom<2048UL, 1, 16> AT24C16;
typedef i2c_eeprom<4096UL, 2, 32> AT24C32;
typedef i2c_eeprom<8192UL, 2, 32> AT24C64;
typedef i2c_eeprom<16384UL, 2, 32> AT24C128;
typedef i2c_eeprom<32768UL, 2, 32> AT24C256;
typedef i2c_eeprom<65536UL, 2, 32> AT24C512;

// STMicroelectronics
typedef i2c_eeprom<128UL, 1, 16> M24C01;
typedef i2c_eeprom<128UL, 1, 8> ST24C01;
typedef i2c_eeprom<256UL, 1, 16> M24C02;
typedef i2c_eeprom<256UL, 1, 8> ST24C02;
typedef i2c_eeprom<512UL, 1, 16> M24C04;
typedef i2c_eeprom<512UL, 1, 8> ST24C04;
typedef i2c_eeprom<1024UL, 1, 16> M24C08;
typedef i2c_eeprom<1024UL, 1, 16> ST24C08;
typedef i2c_eeprom<2048UL, 1, 16> M24C16;
typedef i2c_eeprom<4096UL, 2, 32> ST24C32;
typedef i2c_eeprom<4096UL, 2, 32> M24C32;
typedef i2c_eeprom<8192UL, 2, 32> M24C64;
typedef i2c_eeprom<16384UL, 2, 32> M24128;
typedef i2c_eeprom<32768UL, 2, 32> M24256;
typedef i2c_eeprom<65536UL, 2, 32> M24512;

// Microchip
typedef i2c_eeprom<128UL, 1, 16> MC24C01;
typedef i2c_eeprom<256UL, 1, 8> MC24AA02E48;
typedef i2c_eeprom<256UL, 1, 16> MC24AA025E48;
typedef i2c_eeprom<256UL, 1, 8> MC24AA02UID;
typedef i2c_eeprom<256UL, 1, 16> MC24AA025UID;

} // namespace muonpi::serial::devices::eeproms
#endif // MUONPI_SERIAL_I2CDEVICES_EEPROMS_H
