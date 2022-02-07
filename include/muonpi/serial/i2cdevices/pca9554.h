#ifndef MUONPI_SERIAL_I2CDEVICES_PCA9554_H
#define MUONPI_SERIAL_I2CDEVICES_PCA9554_H
#include "muonpi/serial/i2cbus.h"
//#include "muonpi/serial/i2cdevice.h"
#include "muonpi/serial/i2cdevices/io_extender.h"

namespace muonpi::serial::devices {

class PCA9554 : public io_extender<8> {
public:
    PCA9554(i2c_bus& bus, std::uint8_t address)
        : io_extender<8>(bus,address)
    {
        set_name("PCA9554");
        m_addresses_hint = {
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
            0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
        };
    }
};

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_PCA9554_H