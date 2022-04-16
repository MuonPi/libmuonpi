#ifndef MUONPI_SERIAL_I2CDEVICES_PCA9536_H
#define MUONPI_SERIAL_I2CDEVICES_PCA9536_H

#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevices/pca95xx.h"

#include <optional>

namespace muonpi::serial::devices {

/**
 * @brief The pca9536 class. interact with the PCA9554 8 bit IO extender.
 * This implementation follows the datasheet closely:
 * https://www.ti.com/lit/ds/symlink/pca9536.pdf
 */
class pca9536 : public pca95xx<8> {
public:
    /**
     * @brief pca9536
     * @param bus_traffic The bus traffic object from the i2c_bus.
     * @param path The path of the i2c bus file descriptor
     * @param address The address to use
     */
    pca9536(traffic_t& bus_traffic, const std::string& path, i2c_device::address_type address);

    /**
     * @brief pca9536 Attempts to auto setup the connection
     * @param bus_traffic The bus traffic object from the i2c_bus.
     * @param path The path of the i2c bus file descriptor
     */
    pca9536(traffic_t& bus_traffic, const std::string& path);

    /**
     * @brief identify Attempts to positively identify the device.
     * @return true if identification was successful.
     */
    [[nodiscard]] auto identify() -> bool override;

    constexpr static std::array<address_type, 1> addresses {0b10000011};

    template <std::uint8_t ADDR>
    using register_t = simple_register<std::uint8_t, std::uint8_t, ADDR>;

    struct input_r : public register_t<0x00> {
        constexpr static tag_type register_tag { i2c_register_tag::read };

        const value_type I4_7 : 4 {};
        value_type       I3   : 1 {};
        value_type       I2   : 1 {};
        value_type       I1   : 1 {};
        value_type       I0   : 1 {};

        [[nodiscard]] constexpr auto get() const noexcept -> value_type override {
            return static_cast<value_type>((I3 << 3U) | (I2 << 2U) | (I1 << 1U) | I0);
        }

        constexpr explicit input_r(value_type v) noexcept
            : I3 {static_cast<value_type>((v & 0x08U) >> 3U)}
            , I2 {static_cast<value_type>((v & 0x04U) >> 2U)}
            , I1 {static_cast<value_type>((v & 0x02U) >> 1U)}
            , I0 {static_cast<value_type>(v & 0x01U)} {}

        constexpr explicit input_r() noexcept = default;
    };

    struct output_r : public register_t<0x01> {
        constexpr static tag_type register_tag { i2c_register_tag::read_write };

        const value_type O4_7 : 4 {0xF};
        value_type       O3   : 1 {1};
        value_type       O2   : 1 {1};
        value_type       O1   : 1 {1};
        value_type       O0   : 1 {1};

        [[nodiscard]] constexpr auto get() const noexcept -> value_type override {
            return static_cast<value_type>((O3 << 3U) | (O2 << 2U) | (O1 << 1U) | O0);
        }

        constexpr explicit output_r(value_type v) noexcept
            : O3 {static_cast<value_type>((v & 0x08U) >> 3U)}
            , O2 {static_cast<value_type>((v & 0x04U) >> 2U)}
            , O1 {static_cast<value_type>((v & 0x02U) >> 1U)}
            , O0 {static_cast<value_type>(v & 0x01U)} {}

        constexpr explicit output_r() noexcept = default;
    };

    struct polarity_r : public register_t<0x02> {
        constexpr static tag_type register_tag { i2c_register_tag::read_write };

        constexpr static value_type invert {1};
        constexpr static value_type retain {0};

        const value_type N4_7 : 4 {0x0};
        value_type       N3   : 1 {retain};
        value_type       N2   : 1 {retain};
        value_type       N1   : 1 {retain};
        value_type       N0   : 1 {retain};

        [[nodiscard]] constexpr auto get() const noexcept -> value_type override {
            return static_cast<value_type>((N3 << 3U) | (N2 << 2U) | (N1 << 1U) | N0);
        }

        constexpr explicit polarity_r(value_type v) noexcept
            : N3 {static_cast<value_type>((v & 0x08U) >> 3U)}
            , N2 {static_cast<value_type>((v & 0x04U) >> 2U)}
            , N1 {static_cast<value_type>((v & 0x02U) >> 1U)}
            , N0 {static_cast<value_type>(v & 0x01U)} {}
        constexpr explicit polarity_r() noexcept = default;
    };

    struct configuration_r : public register_t<0x03> {
        constexpr static tag_type register_tag { i2c_register_tag::read_write };

        constexpr static value_type input {1};
        constexpr static value_type output {0};

        const value_type C4_7 : 4 {0xF};
        value_type       C3   : 1 {input};
        value_type       C2   : 1 {input};
        value_type       C1   : 1 {input};
        value_type       C0   : 1 {input};

        [[nodiscard]] constexpr auto get() const noexcept -> value_type override {
            return static_cast<value_type>((C3 << 3U) | (C2 << 2U) | (C1 << 1U) | C0);
        }

        constexpr explicit configuration_r(value_type v) noexcept
            : C3 {static_cast<value_type>((v & 0x08U) >> 3U)}
            , C2 {static_cast<value_type>((v & 0x04U) >> 2U)}
            , C1 {static_cast<value_type>((v & 0x02U) >> 1U)}
            , C0 {static_cast<value_type>(v & 0x01U)} {}
        constexpr explicit configuration_r() noexcept = default;
    };
};

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_PCA9536_H
