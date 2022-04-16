#ifndef MUONPI_SERIAL_I2CDEVICES_LM75_H
#define MUONPI_SERIAL_I2CDEVICES_LM75_H

#include "muonpi/addressrange.h"
#include "muonpi/serial/i2cdevice.h"

namespace muonpi::serial::devices {

/**
 * @brief The lm75 class. Interaction for the LM75 Temperature Sensor.
 * Closely follows the datasheet:
 * https://datasheets.maximintegrated.com/en/ds/LM75.pdf
 */
class lm75 : public i2c_device {
public:
    constexpr static address_range addresses {0b01001000, {0b111}};

    /**
     * @brief lm75
     * @param bus_traffic The bus traffic object from the i2c_bus.
     * @param path The path of the i2c bus file descriptor
     * @param address The address to use
     */
    lm75(traffic_t& bus_traffic, const std::string& path, i2c_device::address_type address);

    /**
     * @brief lm75 Attempts to automatically setup the device connection.
     * @param bus_traffic The bus traffic object from the i2c_bus.
     * @param path The path of the i2c bus file descriptor
     */
    lm75(traffic_t& bus_traffic, const std::string& path);

    /**
     * @brief identify Attempts to positively identify the device.
     * @return True if identification was successful.
     */
    [[nodiscard]] auto identify() -> bool override;

    template <std::uint8_t ADDR>
    struct basic_temperature_r : public simple_register<std::uint16_t, std::uint8_t, ADDR> {
        using value_type = typename simple_register<std::uint16_t, std::uint8_t, ADDR>::value_type;
        using address_type =
            typename simple_register<std::uint16_t, std::uint8_t, ADDR>::address_type;

        value_type       temperature : 9 {};
        const value_type reserved    : 7 {};

        [[nodiscard]] constexpr auto value() const noexcept -> double {
            return static_cast<double>(temperature) / 2.0;
        }

        [[nodiscard]] constexpr auto get() const noexcept -> value_type override {
            return static_cast<value_type>(temperature << 7);
        }

        constexpr explicit basic_temperature_r(value_type v) noexcept
            : temperature {static_cast<value_type>(v >> 7)}
            , reserved {static_cast<value_type>(v & 0b1111111)} {}

        constexpr explicit basic_temperature_r() noexcept = default;
    };

    struct temperature_r : public basic_temperature_r<0x00> {
        constexpr static tag_type register_tag {i2c_register_tag::read};

        constexpr explicit temperature_r(value_type v) noexcept
            : basic_temperature_r {v} {}

        constexpr explicit temperature_r() noexcept = default;
    };

    struct t_hyst_r : public basic_temperature_r<0x01> {
        constexpr static tag_type register_tag {i2c_register_tag::read_write};

        constexpr explicit t_hyst_r(value_type v) noexcept
            : basic_temperature_r {v} {}

        constexpr explicit t_hyst_r() noexcept
            : basic_temperature_r {0b0100101100000000} {}
    };

    struct t_os_r : public basic_temperature_r<0x02> {
        constexpr static tag_type register_tag {i2c_register_tag::read_write};

        constexpr explicit t_os_r(value_type v) noexcept
            : basic_temperature_r {v} {}

        constexpr explicit t_os_r() noexcept
            : basic_temperature_r {0b0101000000000000} {}
    };

    struct configuration_r : public simple_register<std::uint8_t, std::uint8_t, 0x00> {
        constexpr static tag_type register_tag {i2c_register_tag::read_write};

        const value_type reserved    : 3 {};
        value_type       fault_queue : 2 {};
        value_type       os_polarity : 1 {};
        value_type       comparator  : 1 {};
        value_type       shutdown    : 1 {};

        [[nodiscard]] constexpr auto get() const noexcept -> value_type override {
            return static_cast<value_type>((fault_queue << 3) | (os_polarity << 2)
                                           | (comparator << 1) | shutdown);
        }

        constexpr explicit configuration_r(value_type v) noexcept
            : reserved {static_cast<value_type>((v & 0b11100000) >> 5)}
            , fault_queue {static_cast<value_type>((v & 0b11000) >> 3)}
            , os_polarity {static_cast<value_type>((v & 0b100) >> 2)}
            , comparator {static_cast<value_type>((v & 0b10) >> 1)}
            , shutdown {static_cast<value_type>(v & 0b1)} {}

        constexpr explicit configuration_r() noexcept = default;
    };
};

} // namespace muonpi::serial::devices

#endif
