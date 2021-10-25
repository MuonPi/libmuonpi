#include "muonpi/serial/i2cbus.h"

namespace muonpi::serial {

i2c_bus::i2c_bus(std::string address)
    : m_address { std::move(address) }
{
}

i2c_bus::~i2c_bus()
{
    m_devices.clear();
}

auto i2c_bus::address() const -> std::string
{
    return m_address;
}

auto i2c_bus::is_open(std::uint8_t address) const -> bool
{
    const auto iterator = m_devices.find(address);
    if (iterator == m_devices.end()) {
        return false;
    }
    return (iterator->second).get().is_open();
}

auto i2c_bus::close(std::uint8_t address) -> bool
{
    const auto iterator = m_devices.find(address);
    if (iterator == m_devices.end()) {
        return false;
    }

    m_devices.erase(iterator);
    return true;
}

auto i2c_bus::count_devices() const -> std::size_t
{
    return m_devices.size();
}

auto i2c_bus::rx_bytes() const -> std::size_t
{
    return m_rx_bytes;
}

auto i2c_bus::tx_bytes() const -> std::size_t
{
    return m_tx_bytes;
}

auto i2c_bus::get_devices() const -> const std::map<std::uint8_t, std::reference_wrapper<i2c_device>>&
{
    return m_devices;
}
}
