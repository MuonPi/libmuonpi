#ifndef MUONPI_SERIAL_I2CDEVICES_PCA9554_H
#define MUONPI_SERIAL_I2CDEVICES_PCA9554_H

#include "muonpi/multiaddressrange.h"
#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevices/pca95xx.h"

#include <optional>

namespace muonpi::serial::devices {
/**
 * @brief The pca9554 class. interact with the PCA9554 8 bit IO extender.
 * This implementation follows the datasheet closely:
 * https://www.ti.com/lit/ds/symlink/pca9554.pdf
 */
class pca9554 : public pca95xx<8> {
public:
    /**
     * @brief pca9554
     * @param bus_traffic The bus traffic object from the i2c_bus.
     * @param path The path of the i2c bus file descriptor
     * @param address The address to use
     */
    pca9554(traffic_t& bus_traffic, const std::string& path, i2c_device::address_type address);

    /**
     * @brief pca9554 Attempts to auto setup the connection
     * @param bus_traffic The bus traffic object from the i2c_bus.
     * @param path The path of the i2c bus file descriptor
     */
    pca9554(traffic_t& bus_traffic, const std::string& path);

    /**
     * @brief identify Attempts to positively identify the device.
     * @return true if identification was successful.
     */
    [[nodiscard]] auto identify() -> bool override;

    constexpr static multi_address_range addresses {
        {address_range {0b0100000, {0b00000111}}, address_range {0b0111000, {0b00000111}}}};

    template <std::uint8_t ADDR>
    using register_t = simple_register<std::uint8_t, std::uint8_t, ADDR>;

    struct input_r : public register_t<0x00> {
        constexpr static tag_type register_tag { i2c_register_tag::read };

        value_type I7 : 1 {};
        value_type I6 : 1 {};
        value_type I5 : 1 {};
        value_type I4 : 1 {};
        value_type I3 : 1 {};
        value_type I2 : 1 {};
        value_type I1 : 1 {};
        value_type I0 : 1 {};

        [[nodiscard]] constexpr auto get() const noexcept -> value_type override {
            return static_cast<value_type>((I7 << 7U) | (I6 << 6U) | (I5 << 5U) | (I4 << 4U)
                                           | (I3 << 3U) | (I2 << 2U) | (I1 << 1U) | I0);
        }

        constexpr explicit input_r(value_type v) noexcept
            : I7 {static_cast<value_type>((v & 0x80U) >> 7U)}
            , I6 {static_cast<value_type>((v & 0x40U) >> 6U)}
            , I5 {static_cast<value_type>((v & 0x20U) >> 5U)}
            , I4 {static_cast<value_type>((v & 0x10U) >> 4U)}
            , I3 {static_cast<value_type>((v & 0x08U) >> 3U)}
            , I2 {static_cast<value_type>((v & 0x04U) >> 2U)}
            , I1 {static_cast<value_type>((v & 0x02U) >> 1U)}
            , I0 {static_cast<value_type>(v & 0x01U)} {}
        constexpr explicit input_r() noexcept = default;
    };

    struct output_r : public register_t<0x01> {
        constexpr static tag_type register_tag { i2c_register_tag::read_write };

        value_type O7 : 1 {1};
        value_type O6 : 1 {1};
        value_type O5 : 1 {1};
        value_type O4 : 1 {1};
        value_type O3 : 1 {1};
        value_type O2 : 1 {1};
        value_type O1 : 1 {1};
        value_type O0 : 1 {1};

        [[nodiscard]] constexpr auto get() const noexcept -> value_type override {
            return static_cast<value_type>((O7 << 7U) | (O6 << 6U) | (O5 << 5U) | (O4 << 4U)
                                           | (O3 << 3U) | (O2 << 2U) | (O1 << 1U) | O0);
        }

        constexpr explicit output_r(value_type v) noexcept
            : O7 {static_cast<value_type>((v & 0x80U) >> 7U)}
            , O6 {static_cast<value_type>((v & 0x40U) >> 6U)}
            , O5 {static_cast<value_type>((v & 0x20U) >> 5U)}
            , O4 {static_cast<value_type>((v & 0x10U) >> 4U)}
            , O3 {static_cast<value_type>((v & 0x08U) >> 3U)}
            , O2 {static_cast<value_type>((v & 0x04U) >> 2U)}
            , O1 {static_cast<value_type>((v & 0x02U) >> 1U)}
            , O0 {static_cast<value_type>(v & 0x01U)} {}
        constexpr explicit output_r() noexcept = default;
    };

    struct polarity_r : public register_t<0x02> {
        constexpr static tag_type register_tag { i2c_register_tag::read_write };

        constexpr static value_type invert {1};
        constexpr static value_type retain {0};

        value_type N7 : 1 {retain};
        value_type N6 : 1 {retain};
        value_type N5 : 1 {retain};
        value_type N4 : 1 {retain};
        value_type N3 : 1 {retain};
        value_type N2 : 1 {retain};
        value_type N1 : 1 {retain};
        value_type N0 : 1 {retain};

        [[nodiscard]] constexpr auto get() const noexcept -> value_type override {
            return static_cast<value_type>((N7 << 7U) | (N6 << 6U) | (N5 << 5U) | (N4 << 4U)
                                           | (N3 << 3U) | (N2 << 2U) | (N1 << 1U) | N0);
        }

        constexpr explicit polarity_r(value_type v) noexcept
            : N7 {static_cast<value_type>((v & 0x80U) >> 7U)}
            , N6 {static_cast<value_type>((v & 0x40U) >> 6U)}
            , N5 {static_cast<value_type>((v & 0x20U) >> 5U)}
            , N4 {static_cast<value_type>((v & 0x10U) >> 4U)}
            , N3 {static_cast<value_type>((v & 0x08U) >> 3U)}
            , N2 {static_cast<value_type>((v & 0x04U) >> 2U)}
            , N1 {static_cast<value_type>((v & 0x02U) >> 1U)}
            , N0 {static_cast<value_type>(v & 0x01U)} {}
        constexpr explicit polarity_r() noexcept = default;
    };

    struct configuration_r : public register_t<0x03> {
        constexpr static tag_type register_tag { i2c_register_tag::read_write };

        constexpr static value_type input {1};
        constexpr static value_type output {0};

        value_type C7 : 1 {input};
        value_type C6 : 1 {input};
        value_type C5 : 1 {input};
        value_type C4 : 1 {input};
        value_type C3 : 1 {input};
        value_type C2 : 1 {input};
        value_type C1 : 1 {input};
        value_type C0 : 1 {input};

        [[nodiscard]] constexpr auto get() const noexcept -> value_type override {
            return static_cast<value_type>((C7 << 7U) | (C6 << 6U) | (C5 << 5U) | (C4 << 4U)
                                           | (C3 << 3U) | (C2 << 2U) | (C1 << 1U) | C0);
        }

        constexpr explicit configuration_r(value_type v) noexcept
            : C7 {static_cast<value_type>((v & 0x80U) >> 7U)}
            , C6 {static_cast<value_type>((v & 0x40U) >> 6U)}
            , C5 {static_cast<value_type>((v & 0x20U) >> 5U)}
            , C4 {static_cast<value_type>((v & 0x10U) >> 4U)}
            , C3 {static_cast<value_type>((v & 0x08U) >> 3U)}
            , C2 {static_cast<value_type>((v & 0x04U) >> 2U)}
            , C1 {static_cast<value_type>((v & 0x02U) >> 1U)}
            , C0 {static_cast<value_type>(v & 0x01U)} {}
        constexpr explicit configuration_r() noexcept = default;
    };
};

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_PCA9554_H
