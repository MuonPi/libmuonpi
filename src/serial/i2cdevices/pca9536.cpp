#include "muonpi/serial/i2cdevices/pca9536.h"

namespace muonpi::serial::devices {
pca9536::pca9536(traffic_t& bus_traffic, const std::string& path, i2c_device::address_type address)
    : pca95xx<4> {bus_traffic, path, address} {
    set_name("PCA9536");
}

pca9536::pca9536(traffic_t& bus_traffic, const std::string& path)
    : pca95xx<4> {bus_traffic, path, addresses} {
    set_name("PCA9536");
}

auto pca9536::identify() -> bool {
    return identify_impl<pca9536>();
}

} // namespace muonpi::serial::devices
