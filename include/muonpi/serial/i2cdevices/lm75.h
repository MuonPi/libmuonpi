#ifndef MUONPI_SERIAL_I2CDEVICES_LM75_H
#define MUONPI_SERIAL_I2CDEVICES_LM75_H

#include "muonpi/serial/i2cdevice.h"
#include "muonpi/addressrange.h"

namespace muonpi::serial::devices {

/**
 * @brief The lm75 class. Interaction for the LM75 Temperature Sensor.
 * Closely follows the datasheet:
 * https://datasheets.maximintegrated.com/en/ds/LM75.pdf
 */
class lm75 : public i2c_device {
public:
    constexpr static address_range addresses {0b01001000, {0b111}};

    struct temperature_r : public simple_register<> {

    }
};

} // namespace muonpi::serial::devices

#endif
