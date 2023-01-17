#include "muonpi/serial/i2cdevices/pca9554.h"

namespace muonpi::serial::devices {
pca9554::pca9554(traffic_t& bus_traffic, const std::string& path, i2c_device::address_type address)
    : pca95xx<8> {bus_traffic, path, address} {
    set_name("PCA9554");
}

pca9554::pca9554(traffic_t& bus_traffic, const std::string& path)
    : pca95xx<8> {bus_traffic, path, addresses} {
    set_name("PCA9554");
}

auto pca9554::identify() -> bool {
    return identify_impl<pca9554>();
}

} // namespace muonpi::serial::devices
