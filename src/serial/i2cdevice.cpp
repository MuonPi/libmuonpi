#include "muonpi/serial/i2cdevice.h"

#include "muonpi/log.h"
#include "muonpi/scopeguard.h"
#include "muonpi/serial/i2cbus.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <fcntl.h> // open
#include <iostream>
#include <linux/i2c-dev.h> // I2C bus definitions for linux like systems

namespace muonpi::serial {

i2c_device::i2c_device(i2c_bus& bus, std::uint8_t address)
    : m_bus { bus }
    , m_address { address }
    , m_handle { open(bus.address().c_str(), O_RDWR) }
{
    if (m_handle > 0) {
        set_address(m_address);
    } else {
        set_flag(Flags::Failed);
    }
}

i2c_device::i2c_device(i2c_bus& bus)
    : m_bus { bus }
    , m_handle { open(bus.address().c_str(), O_RDWR) }
{
    if (m_handle > 0) {
        // no address to set yet
    } else {
        set_flag(Flags::Failed);
    }
}

void i2c_device::set_address(std::uint8_t address)
{
    if (m_handle > 0) {
        if (ioctl(m_handle, I2C_SLAVE, m_address) < 0) {
            if (ioctl(m_handle, I2C_SLAVE_FORCE, m_address) < 0) {
                m_io_errors++;
                set_flag(Flags::Failed);
            } else {
                set_flag(Flags::Force);
            }
        } else {
            set_flag(Flags::Normal);
        }
    } else {
        set_flag(Flags::Failed);
    }
    m_address = address;
}

i2c_device::~i2c_device()
{
    close();
}

auto i2c_device::is_open() const -> bool
{
    return m_handle > 0;
}

void i2c_device::close() const
{
    if (m_handle > 0) {
        ::close(m_handle);
    }
}

void i2c_device::read_capabilities() const
{
    unsigned long funcs {};
    int res = ioctl(m_handle, I2C_FUNCS, &funcs);
    if (res < 0) {
        log::error("i2c") << "Could not read i2c device capabilities";
    } else {
        log::info("i2c") << "Device capabilities: 0x" << std::hex << funcs;
    }
}

auto i2c_device::present() -> bool
{
    uint8_t dummy {};
    return (read(&dummy, 1) == 1);
}

auto i2c_device::identify() -> bool
{
    return false;
}

auto i2c_device::io_errors() const -> std::size_t
{
    return m_io_errors;
}

auto i2c_device::rx_bytes() const -> std::size_t
{
    return m_rx_bytes;
}

auto i2c_device::tx_bytes() const -> std::size_t
{
    return m_tx_bytes;
}

auto i2c_device::flag_set(Flags flag) const -> bool
{
    return (m_flags & static_cast<std::uint8_t>(flag)) > 0;
}

void i2c_device::set_flag(Flags flag)
{
    m_flags |= static_cast<std::uint8_t>(flag);
}

void i2c_device::unset_flag(Flags flag)
{
    // the following insane statement mutes the HIC++ standard "hicpp-signed-bitwise" violation
    // warning from clang-tidy details on
    // https://stackoverflow.com/questions/50399090/use-of-a-signed-integer-operand-with-a-binary-bitwise-operator-when-using-un
    m_flags &= ~static_cast<std::uint8_t>(flag); // NOLINT(hicpp-signed-bitwise)
}

void i2c_device::lock(bool locked)
{
    m_locked = locked;
}

auto i2c_device::locked() const -> bool
{
    return m_locked;
}

auto i2c_device::last_interval() const -> double
{
    return 1e-3 * m_last_duration.count();
}

auto i2c_device::last_access_duration() const -> std::chrono::microseconds
{
    return m_last_duration;
}

void i2c_device::set_name(std::string name)
{
    m_name = std::move(name);
}

auto i2c_device::name() const -> std::string
{
    return m_name;
}

auto i2c_device::addresses_hint() const -> const std::set<std::uint8_t>&
{
    return m_addresses_hint;
}

void i2c_device::set_addresses_hint(std::set<std::uint8_t> address_list)
{
    m_addresses_hint = std::move(address_list);
}

auto i2c_device::read(std::uint8_t* buffer, std::size_t bytes) -> int
{
    if (locked() || (m_handle <= 0)) {
        return 0;
    }
    int nread = ::read(m_handle, buffer, bytes);
    if (nread > 0) {
        m_rx_bytes += nread;
        m_bus.m_rx_bytes += nread;
        unset_flag(Flags::Unreachable);
    } else {
        m_io_errors++;
        set_flag(Flags::Unreachable);
    }
    return nread;
}

auto i2c_device::read(std::uint8_t reg, std::uint8_t bit_mask) -> std::uint16_t
{
    std::uint8_t buffer {};
    if (read(reg, &buffer, 1) != 1) {
        return 0xFFFF;
    }
    return buffer & bit_mask;
}

auto i2c_device::read(std::uint8_t reg, std::uint8_t* buffer, std::size_t bytes) -> int
{
    if (write(&reg, 1) != 1) {
        return -1;
    }
    return read(buffer, bytes);
}

auto i2c_device::read(std::uint8_t reg, std::uint16_t* buffer, std::size_t n_words) -> int
{
    if (write(&reg, 1) != 1) {
        return -1;
    }

    auto read_buffer = std::make_unique<std::uint8_t[]>(n_words * 2u);

    int nread = read(read_buffer.get(), n_words * 2u);

    if (nread != static_cast<int>(n_words) * 2) {
        return -1;
    }

    for (std::size_t i { 0 }; i < n_words; ++i) {
        buffer[i] = read_buffer[i * 2u] << 8u;
        buffer[i] |= read_buffer[i * 2u + 1];
    }

    return nread / 2;
}

auto i2c_device::read(std::uint8_t reg, std::uint16_t bit_mask) -> std::uint32_t
{
    std::uint16_t buffer {};
    if (read(reg, &buffer, 1) != 1) {
        return 0xFFFFFFFF;
    }
    return buffer & bit_mask;
}

auto i2c_device::write(std::uint8_t* buffer, std::size_t bytes) -> int
{
    if (locked() || (m_handle <= 0)) {
        return 0;
    }
    int nwritten = ::write(m_handle, buffer, bytes);
    if (nwritten > 0) {
        m_tx_bytes += nwritten;
        m_bus.m_tx_bytes += nwritten;
        unset_flag(Flags::Unreachable);
    } else {
        m_io_errors++;
        set_flag(Flags::Unreachable);
    }
    return nwritten;
}

auto i2c_device::write(std::uint8_t reg, std::uint8_t bit_mask, std::uint8_t value) -> bool
{
    std::uint8_t buffer {};
    if (read(reg, &buffer, 1) != 1) {
        return false;
    }
    buffer = (buffer & ~bit_mask) | (value & bit_mask);

    return write(reg, &buffer, 1) == 1;
}

auto i2c_device::write(std::uint8_t reg, std::uint8_t* buffer, std::size_t bytes) -> int
{
    auto write_buffer = std::make_unique<std::uint8_t[]>(bytes + 1u);

    write_buffer[0] = reg;

    std::memcpy(write_buffer.get() + 1, buffer, bytes);

    return write(write_buffer.get(), bytes + 1) - 1;
}

auto i2c_device::write(std::uint8_t reg, const std::uint16_t* buffer, std::size_t length) -> int
{
    auto write_buffer = std::make_unique<std::uint8_t[]>(length * 2u + 1u);

    for (std::size_t i { 0 }; i < length; i++) {
        write_buffer[i * 2u] = buffer[i] >> 8u;
        write_buffer[i * 2u + 1u] = buffer[i];
    }

    int count = write(reg, write_buffer.get(), length * 2u);

    if (count < 0) {
        return -1;
    }
    return count / 2;
}

void i2c_device::start_timer()
{
    m_start = std::chrono::system_clock::now();
}

void i2c_device::stop_timer()
{
    m_last_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now() - m_start);
}

auto i2c_device::setup_timer() -> scope_guard
{
    start_timer();
    return scope_guard { [&] { stop_timer(); } };
}

} // namespace muonpi::serial
