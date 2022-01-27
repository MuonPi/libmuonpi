#ifndef MUONPI_SERIAL_I2CDEVICES_MIC184_H
#define MUONPI_SERIAL_I2CDEVICES_MIC184_H
#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"

namespace muonpi::serial::devices {

class MIC184 : public i2c_device {
public:
    explicit MIC184(i2c_bus& bus, std::uint8_t address);
    ~MIC184() override;

    [[nodiscard]] auto get_temperature() -> float;
    [[nodiscard]] auto identify() -> bool override;
    [[nodiscard]] auto is_external() const -> bool;
    [[nodiscard]] auto set_external(bool enable_external = true) -> bool;

private:
    [[nodiscard]] auto readRaw() -> int16_t;

    enum REG : uint8_t {
        TEMP = 0x00,
        CONF = 0x01,
        THYST = 0x02,
        TOS = 0x03
    };

    bool fExternal { false };
};

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_MIC184_H
