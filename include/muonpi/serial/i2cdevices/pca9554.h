#ifndef MUONPI_SERIAL_I2CDEVICES_PCA9554_H
#define MUONPI_SERIAL_I2CDEVICES_PCA9554_H
#include "muonpi/serial/i2cbus.h"
//#include "muonpi/serial/i2cdevice.h"
#include "muonpi/serial/i2cdevices/io_extender.h"

namespace muonpi::serial::devices {

class PCA9554 : public io_extender<8> {
public:
    PCA9554(i2c_bus& bus, std::uint8_t address = InvalidI2cAddress)
        : io_extender<8>(bus, address) {
        set_name("PCA9554");
    }
};

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_PCA9554_H
