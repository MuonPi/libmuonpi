#include "muonpi/serial/i2cdevices/lm75.h"

namespace muonpi::serial::devices {

lm75::lm75(traffic_t& bus_traffic, const std::string& path, i2c_device::address_type address)
    : i2c_device { bus_traffic, path, address} {}

lm75::lm75(traffic_t& bus_traffic, const std::string& path)
    : i2c_device { bus_traffic, path, addresses} {}



auto lm75::identify() -> bool {
    if (flag_set(Flags::Error)) {
        return false;
    }
    if (!present()) {
        return false;
    }

    const auto config { read<configuration_r>() };
    if (!config.has_value()) {
        return false;
    }
    if (config.value().reserved != 0) {
        return false;
    }
    const auto temp { read<temperature_r>() };
    if (!temp.has_value()) {
        return false;
    }
    if (temp.value().reserved != 0) {
        return false;
    }
    const auto thyst { read<t_hyst_r>() };
    if (!thyst.has_value()) {
        return false;
    }
    if (thyst.value().reserved != 0) {
        return false;
    }
    const auto tos { read<t_os_r>() };
    if (!tos.has_value()) {
        return false;
    }
    if (tos.value().reserved != 0) {
        return false;
    }

    return true;
}
} // namespace muonpi::serial::devices
