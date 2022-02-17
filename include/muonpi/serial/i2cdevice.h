#ifndef MUONPI_SERIAL_I2CDEVICE_H
#define MUONPI_SERIAL_I2CDEVICE_H

#include "muonpi/scopeguard.h"

#include <chrono>
#include <cinttypes> // std::uint8_t, etc
#include <fcntl.h>   // open
#include <iostream>
#include <set>
#include <string>
#include <sys/ioctl.h> // ioctl
#include <vector>

namespace muonpi::serial {

class i2c_bus;

class i2c_device {
public:
    enum class Flags : std::uint8_t
    {
        None        = 0,
        Normal      = 0x01,
        Force       = 0x02,
        Unreachable = 0x04,
        Failed      = 0x08,
        Locked      = 0x10
    };

    i2c_device(i2c_bus& bus, std::uint8_t address);
    explicit i2c_device(i2c_bus& bus);

    void               set_address(std::uint8_t address);
    [[nodiscard]] auto address() const -> std::uint8_t {
        return m_address;
    }

    virtual ~i2c_device();

    [[nodiscard]] auto is_open() const -> bool;
    void               close() const;

    void                       read_capabilities() const;
    [[nodiscard]] virtual auto present() -> bool;
    [[nodiscard]] virtual auto identify() -> bool;

    [[nodiscard]] auto io_errors() const -> std::size_t;

    [[nodiscard]] auto rx_bytes() const -> std::size_t;
    [[nodiscard]] auto tx_bytes() const -> std::size_t;

    [[nodiscard]] auto flag_set(Flags flag) const -> bool;

    void               lock(bool locked = true);
    [[nodiscard]] auto locked() const -> bool;

    [[deprecated]] [[nodiscard]] auto last_interval() const -> double;
    [[nodiscard]] auto                last_access_duration() const -> std::chrono::microseconds;

    void               set_name(std::string name);
    [[nodiscard]] auto name() const -> std::string;

    [[nodiscard]] auto addresses_hint() const -> const std::set<std::uint8_t>&;

    [[nodiscard]] auto read(std::uint8_t* buffer, std::size_t bytes = 1) -> int;

    [[nodiscard]] auto read(std::uint8_t reg, std::uint8_t bit_mask) -> std::uint16_t;

    [[nodiscard]] auto read(std::uint8_t reg, std::uint16_t bit_mask) -> std::uint32_t;

    [[nodiscard]] auto read(std::uint8_t reg, std::uint8_t* buffer, std::size_t bytes = 1) -> int;

    [[nodiscard]] auto read(std::uint8_t reg, std::uint16_t* buffer, std::size_t n_words = 1)
        -> int;

    auto write(std::uint8_t* buffer, std::size_t bytes = 1) -> int;

    auto write(std::uint8_t reg, std::uint8_t bit_mask, std::uint8_t value) -> bool;

    auto write(std::uint8_t reg, std::uint8_t* buffer, std::size_t bytes = 1) -> int;

    auto write(std::uint8_t reg, const std::uint16_t* buffer, std::size_t length = 1) -> int;

protected:
    void set_flag(Flags flag);
    void unset_flag(Flags flag);

    void start_timer();
    void stop_timer();

    [[nodiscard]] auto setup_timer() -> scope_guard;

    std::set<std::uint8_t> m_addresses_hint {};

private:
    i2c_bus& m_bus;

    static constexpr std::uint8_t s_default_address {0xff};

    std::uint8_t m_address {s_default_address};

    int  m_handle {};
    bool m_locked {false};

    std::size_t m_rx_bytes {};
    std::size_t m_tx_bytes {};

    std::size_t m_io_errors {};
    std::chrono::microseconds
        m_last_duration {}; // the last time measurement's result is stored here

    std::string  m_name {"I2C device"};
    std::uint8_t m_flags {};

    std::chrono::system_clock::time_point m_start {};
};

} // namespace muonpi::serial

#endif // MUONPI_SERIAL_I2CDEVICE_H
