#include "muonpi/serial/i2cbus.h"

namespace muonpi::serial {
i2c_bus::i2c_bus(std::string path)
    : m_path {std::move(path)} {}

i2c_bus::i2c_bus() = default;

i2c_bus::~i2c_bus() {
    m_devices.clear();
}

auto i2c_bus::path() const -> std::string {
    return m_path;
}

auto i2c_bus::close(std::uint8_t address) -> bool {
    const auto iterator = m_devices.find(address);
    if (iterator == m_devices.end()) {
        return false;
    }
    m_devices.erase(iterator);
    return true;
}

auto i2c_bus::count_devices() const -> std::size_t {
    return m_devices.size();
}

auto i2c_bus::rx_bytes() const -> std::size_t {
    return m_traffic_data.rx_bytes;
}

auto i2c_bus::tx_bytes() const -> std::size_t {
    return m_traffic_data.tx_bytes;
}

auto i2c_bus::get_devices() const -> const std::map<std::uint8_t, std::shared_ptr<i2c_device>>& {
    return m_devices;
}
} // namespace muonpi::serial
