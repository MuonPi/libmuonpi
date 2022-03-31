#ifndef MUONPI_SERIAL_I2CDEVICES_MIC184_H
#define MUONPI_SERIAL_I2CDEVICES_MIC184_H
#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"

namespace muonpi::serial::devices {

/**
 * @brief I2C temperature sensor device class.
 * This class provides access to the temperature measurement for MIC184 i2c temperature sensors.
 * The temperature value (in degrees Celsius) is returned by the @link #get_temperature
 * get_temperature @endlink method. Furthermore, switching between internal and external zone is
 * supported. For temperature measurements in the external zone, a sense device must be connected
 * according to the data sheet:
 * https://ww1.microchip.com/downloads/en/DeviceDoc/MIC184-Local-Remote-Thermal-Supervisor-DS20006457A.pdf
 */
class MIC184 : public i2c_device, public static_device_base<MIC184> {
public:
    explicit MIC184(i2c_bus& bus, std::uint8_t address = InvalidI2cAddress);

    ~MIC184() override;

    /**
     * @brief get the current reading for the temperature
     * @return the temperature in degrees Celsius
     */
    [[nodiscard]] auto get_temperature() -> float;
    [[nodiscard]] auto identify() -> bool override;

    /**
     * @brief get the status of the current temperature zone (internal or external)
     * @return true, if the external zone is selected
     */
    [[nodiscard]] auto is_external() const -> bool;

    /**
     * @brief set temperature zone (internal or external)
     * @param enable_external true for external zone, false for internal
     * @return true, if the zone could be set in the device
     */
    [[nodiscard]] auto set_external(bool enable_external = true) -> bool;

private:
    [[nodiscard]] auto readRaw() -> std::int16_t;

    enum REG : std::uint8_t
    {
        TEMP  = 0x00,
        CONF  = 0x01,
        THYST = 0x02,
        TOS   = 0x03
    };

    bool m_external {false};
};

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_MIC184_H
