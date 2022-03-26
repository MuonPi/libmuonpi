#include "muonpi/serial/i2cbus.h"

namespace muonpi::serial {

i2c_bus::general_call_t::general_call_t(i2c_bus* bus)
    : m_bus(bus) {}

auto i2c_bus::general_call_t::reset() -> bool {
    if (m_bus == nullptr) {
        return false;
    }
    i2c_device   dev {*m_bus, GeneralCallAddress};
    std::uint8_t data {0x06};
    return (dev.write(&data, 1) == 1);
}

auto i2c_bus::general_call_t::wake_up() -> bool {
    if (m_bus == nullptr) {
        return false;
    }
    i2c_device   dev {*m_bus, GeneralCallAddress};
    std::uint8_t data {0x09};
    return (dev.write(&data, 1) == 1);
}

auto i2c_bus::general_call_t::software_update() -> bool {
    if (m_bus == nullptr) {
        return false;
    }
    i2c_device   dev {*m_bus, GeneralCallAddress};
    std::uint8_t data {0x08};
    return (dev.write(&data, 1) == 1);
}

i2c_bus::i2c_bus(std::string address)
    : general_call(this)
    , m_address {std::move(address)} {}

i2c_bus::i2c_bus() = default;

i2c_bus::~i2c_bus() {
    m_devices.clear();
}

auto i2c_bus::address() const -> std::string {
    return m_address;
}

auto i2c_bus::is_open(std::uint8_t address) const -> bool {
    const auto iterator = m_devices.find(address);
    if (iterator == m_devices.end()) {
        return false;
    }
    return (iterator->second)->is_open();
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
    return m_rx_bytes;
}

auto i2c_bus::tx_bytes() const -> std::size_t {
    return m_tx_bytes;
}

auto i2c_bus::get_devices() const -> const std::map<std::uint8_t, std::shared_ptr<i2c_device>>& {
    return m_devices;
}

} // namespace muonpi::serial
