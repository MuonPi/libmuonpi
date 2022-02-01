#ifndef MUONPI_SERIAL_I2CDEVICES_PCA9536_H
#define MUONPI_SERIAL_I2CDEVICES_PCA9536_H
#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"

#include <optional>

namespace muonpi::serial::devices {

/* PCA9536  */
class PCA9536 : public i2c_device {
    // the device supports reading the incoming logic levels of the pins if set to input in the configuration register (will probably not use this feature)
    // the device supports polarity inversion (by configuring the polarity inversion register) (will probably not use this feature)
public:
    enum PORT : std::uint8_t {
        CH0 = 0,
        CH1 = 2,
        CH2 = 4,
        CH3 = 8
    };

    PCA9536(i2c_bus& bus, std::uint8_t address);

    [[nodiscard]] auto set_direction_mask(std::uint8_t output_mask) -> bool;
    [[nodiscard]] auto set_output_states(std::uint8_t state_mask) -> bool;
    [[nodiscard]] auto get_input_states() -> std::optional<std::uint8_t>;

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
