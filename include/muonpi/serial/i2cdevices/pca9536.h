#ifndef MUONPI_SERIAL_I2CDEVICES_PCA9536_H
#define MUONPI_SERIAL_I2CDEVICES_PCA9536_H
#include "muonpi/serial/i2cbus.h"
//#include "muonpi/serial/i2cdevice.h"
#include "muonpi/serial/i2cdevices/io_extender.h"

namespace muonpi::serial::devices {

class PCA9536 : public io_extender<4> {
public:
    PCA9536(i2c_bus& bus, std::uint8_t address = InvalidI2cAddress)
        : io_extender<4>(bus, address) {
        set_name("PCA9536");
    }
};

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_PCA9536_H
