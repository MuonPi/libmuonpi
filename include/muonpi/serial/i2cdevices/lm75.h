#ifndef MUONPI_SERIAL_I2CDEVICES_LM75_H
#define MUONPI_SERIAL_I2CDEVICES_LM75_H
#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"

namespace muonpi::serial::devices {

class LM75 : public i2c_device {
public:
    LM75(i2c_bus& bus, std::uint8_t address);

    ~LM75() override;

    [[nodiscard]] auto get_temperature() -> float;
    [[nodiscard]] auto identify() -> bool override;

protected:
    [[nodiscard]] auto readRaw() -> std::int16_t;

    enum REG : uint8_t {
        TEMP  = 0x00,
        CONF  = 0x01,
        THYST = 0x02,
        TOS   = 0x03
    };
};

} // namespace muonpi::serial::devices

#endif // MUONPI_SERIAL_I2CDEVICES_LM75_H
