#include "muonpi/serial/i2cdevice.h"

#include "muonpi/scopeguard.h"
#include "muonpi/serial/i2cbus.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <linux/i2c-dev.h>

namespace muonpi::serial {
i2c_device::i2c_device(traffic_t& bus_traffic, const std::string& path, address_type address)
    : m_bus_traffic {bus_traffic}
    , m_address {address}
    , m_handle {open(path.c_str(), O_RDWR)} // NOLINT: cppcoreguidelines-pro-type-vararg
{
    if ((m_handle <= 0) || !set_address(m_address)) {
        set_flag(Flags::Failed);
        throw std::runtime_error {
            ("Could not initialise I2C device '" + path + " : " + std::to_string(address) + "'")
                .c_str()};
    }
}

auto i2c_device::set_address(address_type address) -> bool {
    scope_guard guard {[&] { set_flag(Flags::Failed); }, [&] { m_address = address; }};

    if (m_handle <= 0) {
        return false;
    }
    // clang-format off
    if (ioctl(m_handle, I2C_SLAVE, m_address) >= 0) { // NOLINT(cppcoreguidelines-pro-type-vararg,hicpp-vararg)
        // clang-format on
        set_flag(Flags::Normal);
        guard.dismiss();
        m_address = address;
        return true;
    }
    // clang-format off
    if (ioctl(m_handle, I2C_SLAVE_FORCE, m_address) >= 0) { // NOLINT(cppcoreguidelines-pro-type-vararg,hicpp-vararg)
        // clang-format on
        set_flag(Flags::Force);
        guard.dismiss();
        m_address = address;
        return true;
    }
    return false;
}

auto i2c_device::address() const -> address_type {
    return m_address;
}

i2c_device::~i2c_device() {
    close();
}

auto i2c_device::is_open() const -> bool {
    return m_handle > 0;
}

void i2c_device::close() {
    if (m_handle > 0) {
        ::close(m_handle);
        m_handle = 0;
    }
}

void i2c_device::read_capabilities() const {
    unsigned long funcs {};
    ioctl(m_handle, I2C_FUNCS, &funcs); // NOLINT(cppcoreguidelines-pro-type-vararg,hicpp-vararg)
}

auto i2c_device::present() -> bool {
    if (!writable()) {
        return false;
    }
    return read<std::uint8_t>().has_value();
}

auto i2c_device::identify() -> bool {
    return false;
}

auto i2c_device::io_errors() const -> std::size_t {
    return m_io_errors;
}

auto i2c_device::rx_bytes() const -> std::size_t {
    return m_traffic.rx_bytes;
}

auto i2c_device::tx_bytes() const -> std::size_t {
    return m_traffic.tx_bytes;
}

auto i2c_device::flag_set(Flags flag) const -> bool {
    return (m_flags & static_cast<std::uint8_t>(flag)) > 0;
}

void i2c_device::set_flag(Flags flag) {
    m_flags |= static_cast<std::uint8_t>(flag);
    if ((m_flags & static_cast<std::uint8_t>(Flags::Error)) > 0) {
        m_io_errors++;
    }
}

void i2c_device::unset_flag(Flags flag) {
    // casting twice:
    // the inner cast converts the Flag enumeration to `std::uint8_t`
    // The operator ~ undergoes Integral Promotion to `int`:
    // https://en.cppreference.com/w/cpp/language/implicit_conversion#Integral_promotion So it gets
    // cast to `std::uint8_t` a second time.
    m_flags &= static_cast<std::uint8_t>(~static_cast<std::uint8_t>(flag));
}

void i2c_device::lock(bool locked) {
    m_locked = locked;
    if (m_locked) {
        set_flag(Flags::Locked);
    } else {
        unset_flag(Flags::Locked);
    }
}

auto i2c_device::locked() const -> bool {
    return m_locked;
}

void i2c_device::set_name(std::string name) {
    m_name = std::move(name);
}

auto i2c_device::name() const -> std::string {
    return m_name;
}

auto i2c_device::writable() const -> bool {
    return is_open() && !locked();
}

void i2c_device::add_rx_bytes(std::size_t bytes) {
    m_traffic.rx_bytes += bytes;
    m_bus_traffic.rx_bytes += bytes;
}

void i2c_device::add_tx_bytes(std::size_t bytes) {
    m_traffic.tx_bytes += bytes;
    m_bus_traffic.tx_bytes += bytes;
}

} // namespace muonpi::serial
