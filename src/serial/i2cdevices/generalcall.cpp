#include "muonpi/serial/i2cdevices/generalcall.h"

namespace muonpi::serial {

general_call::general_call(traffic_t& traffic, const std::string& path)
    : i2c_device {traffic, path, addresses[0]} {}

auto general_call::reset() -> bool {
    return write<std::uint8_t>(0x06);
}
auto general_call::wake_up() -> bool {
    return write<std::uint8_t>(0x09);
}

auto general_call::firmware_update() -> bool {
    return write<std::uint8_t>(0x08);
}

} // namespace muonpi::serial
