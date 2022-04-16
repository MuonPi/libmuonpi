#ifndef MUONPI_SERIAL_I2CDEVICES_PCA95XX_H
#define MUONPI_SERIAL_I2CDEVICES_PCA95XX_H
#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"

#include <optional>

namespace muonpi::serial::devices {

template <std::size_t B>
concept bits_c = ((B == 4) || (B == 8));

template <std::size_t BITS>
requires bits_c<BITS>
    /**
     * @brief I2C io extender device class.
     * This class provides access to i2c 4-bit and 8-bit bidirectional digital i/o
     * extenders.
     * @note valid template specializations are available for values of the template
     * parameter BITS of 4 and 8.
     */
    class pca95xx : public i2c_device {
public:
    static constexpr std::size_t width {BITS};

protected:
    pca95xx(traffic_t& bus_traffic, const std::string& path, i2c_device::address_type address);
    pca95xx(traffic_t&                   bus_traffic,
            const std::string&           path,
            const address_iterable auto& addresses);

    template <std::derived_from<pca95xx<BITS>> T>
    [[nodiscard]] auto identify_impl() -> bool {
        if (flag_set(Flags::Error)) {
            return false;
        }
        if (!present()) {
            return false;
        }

        const auto input {read<typename T::input_r>()};
        if (!input.has_value()) {
            return false;
        }
        const auto output {read<typename T::output_r>()};
        if (!output.has_value()) {
            return false;
        }
        if (output.value().get() != typename T::output_r {}.get()) {
            return false;
        }
        const auto polarity {read<typename T::polarity_r>()};
        if (!polarity.has_value()) {
            return false;
        }
        if (polarity.value().get() != typename T::polarity_r {}.get()) {
            return false;
        }
        const auto configuration {read<typename T::configuration_r>()};
        if (!configuration.has_value()) {
            return false;
        }
        if (configuration.value().get() != typename T::configuration_r {}.get()) {
            return false;
        }
        return true;
    }
};

/***************************
 * Implementation part
 ***************************/

template <std::size_t BITS>
requires bits_c<BITS> pca95xx<BITS>::pca95xx(traffic_t&               bus_traffic,
                                             const std::string&       path,
                                             i2c_device::address_type address)
    : i2c_device {bus_traffic, path, address} {}

template <std::size_t BITS>
requires bits_c<BITS> pca95xx<BITS>::pca95xx(traffic_t&                   bus_traffic,
                                             const std::string&           path,
                                             const address_iterable auto& addresses)
    : i2c_device {bus_traffic, path, addresses} {}

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_PCA95XX_H
