#ifndef MUONPI_SERIAL_I2CDEVICES_PCA9536_H
#define MUONPI_SERIAL_I2CDEVICES_PCA9536_H
#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"

namespace muonpi::serial::devices {

/* PCA9536  */
class PCA9536 : public i2c_device {
    // the device supports reading the incoming logic levels of the pins if set to input in the configuration register (will probably not use this feature)
    // the device supports polarity inversion (by configuring the polarity inversion register) (will probably not use this feature)
public:
    enum CFG_PORT {
        C0 = 0,
        C1 = 2,
        C3 = 4,
        C4 = 8
    };

    PCA9536(i2c_bus& bus, std::uint8_t address);

    [[nodiscard]] auto setOutputPorts(std::uint8_t portMask) -> bool;
    [[nodiscard]] auto setOutputState(std::uint8_t portMask) -> bool;
    [[nodiscard]] auto getInputState(std::uint8_t* state) -> bool; // TODO: Don't use output parameters and bool return value. Use std::optional<std::uint8_t> or similar instead. Especially don't use pointers as output parameter

    [[nodiscard]] auto present() -> bool override;
    [[nodiscard]] auto identify() -> bool override;

private:
    enum REG {
        INPUT = 0x00,
        OUTPUT = 0x01,
        POLARITY = 0x02,
        CONFIG = 0x03
    };
};

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_PCA9536_H
