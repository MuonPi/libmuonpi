#include "muonpi/serial/i2cdevice.h"
#include "muonpi/serial/i2cbus.h"
#include "muonpi/scopeguard.h"

#include "muonpi/log.h"


#include <algorithm>
#include <cstring>
#include <fcntl.h> // open
#include <iostream>
#include <linux/i2c-dev.h> // I2C bus definitions for linux like systems


namespace muonpi::serial {


i2c_device::i2c_device(const i2c_bus& bus, std::size_t& rx_counter, std::size_t& tx_counter, std::uint8_t address)
    : m_address { address }
    , m_handle { open(bus.address().c_str(), O_RDWR) }
    , m_rx_counter { rx_counter }
    , m_tx_counter { tx_counter }
{
    if (m_handle > 0) {
        if (ioctl(m_handle, I2C_SLAVE, m_address) < 0) {
            if (ioctl(m_handle, I2C_SLAVE_FORCE, m_address) < 0) {
                m_io_errors++;
            } else {
                set_flag(Flags::Force);
            }
        } else {
            set_flag(Flags::Normal);
        }
    } else {
        set_flag(Flags::Failed);
    }
}



i2c_device::~i2c_device()
{
    close();
}

auto i2c_device::is_open() const -> bool
{
    return m_handle > 0;
}

void i2c_device::close()
{
    if (m_handle > 0) {
        ::close(m_handle);
    }
}


void i2c_device::read_capabilities()
{
    unsigned long funcs;
    int res = ioctl(m_handle, I2C_FUNCS, &funcs);
    if (res < 0) {
        log::error("i2c") << "Could not read i2c device capabilities";
    } else {
        log::info("i2c") << "Device capabilities: 0x"<<std::hex<<funcs;
    }
}

auto i2c_device::present() -> bool
{
    uint8_t dummy;
    return (read(&dummy, 1) == 1);
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
    m_flags &= ~static_cast<std::uint8_t>(flag);
}

void i2c_device::lock(bool locked)
{
    m_locked = locked;
}

auto i2c_device::last_interval() const -> double
{
    return m_last_interval;
}

void i2c_device::set_title(std::string title)
{
    m_title = std::move(title);
}

auto i2c_device::title() const -> std::string
{
    return m_title;
}


auto i2c_device::read(std::uint8_t* buffer, std::size_t bytes) -> int
{
    if (locked() || (m_handle <= 0)) {
        return 0;
    }
    int nread = ::read(m_handle, buffer, bytes);
    if (nread > 0) {
        m_rx_bytes += nread;
        m_rx_counter += nread;
        unset_flag(Flags::Unreachable);
    } else {
        m_io_errors++;
        set_flag(Flags::Unreachable);
    }
    return nread;
}

auto i2c_device::read(std::uint8_t reg, std::uint8_t bit_mask) -> std::uint16_t
{
    uint8_t buffer {};
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

auto i2c_device::write(uint8_t* buffer, std::size_t bytes) -> int
{
    if (locked() || (m_handle <= 0)) {
        return 0;
    }
    int nwritten = ::write(m_handle, buffer, bytes);
    if (nwritten > 0) {
        m_tx_bytes += nwritten;
        m_tx_counter += nwritten;
        unset_flag(Flags::Unreachable);
    } else {
        m_io_errors++;
        set_flag(Flags::Unreachable);
    }
    return nwritten;
}

auto i2c_device::write(std::uint8_t reg, std::uint8_t bit_mask, std::uint8_t value) -> bool
{
    uint8_t buffer {};
    if (read(reg, &buffer, 1) != 1) {
        return false;
    }
    buffer = (buffer & ~bit_mask) | (value & bit_mask);

    return write(reg, &buffer, 1) == 1;
}

auto i2c_device::write(std::uint8_t reg, std::uint8_t* buffer, std::size_t bytes) -> int
{
    std::uint8_t* write_buffer { static_cast<std::uint8_t*>(calloc(sizeof(std::uint8_t), bytes + 1)) };

    scope_guard free_guard { [&write_buffer]{
            free(write_buffer);
        } };

    write_buffer[0] = reg;

    std::memcpy(write_buffer + 1, buffer, bytes);

    return write(write_buffer, bytes + 1) - 1;
}



auto i2c_device::write(std::uint8_t reg, uint16_t* buffer, std::size_t length) -> int
{
    std::uint8_t* write_buffer { static_cast<std::uint8_t*>(calloc(sizeof(std::uint8_t), length * 2 + 1)) };

    scope_guard free_guard { [&write_buffer]{
            free(write_buffer);
        } };

    for (std::size_t i { 0 }; i < length; i++) {
        write_buffer[i * 2] = buffer[i] >> 8;
        write_buffer[i * 2 + 1] = buffer[i];
    }

    int count = write(reg, write_buffer, length * 2);

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
    m_last_interval = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - m_start).count())*1e-3;
}
}
