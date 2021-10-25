#ifndef MUONPI_I2CINTERFACE_H
#define MUONPI_I2CINTERFACE_H

#include "muonpi/serial/i2cdevice.h"

#include <fcntl.h> // open
#include <sys/ioctl.h> // ioctl
#include <sys/time.h> // for gettimeofday()
#include <unistd.h>
#include <stdio.h>

#include <cinttypes>

#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <type_traits>


namespace muonpi::serial {

class i2c_bus {

public:
    explicit i2c_bus(std::string address);

    virtual ~i2c_bus();

    [[nodiscard]] auto address() const -> std::string;

    template <typename T, std::enable_if_t<std::is_base_of<i2c_device, T>::value, bool> = true>
    [[nodiscard]] auto open(std::uint8_t address) -> T&
    {
        m_devices.emplace(address, T {*this, m_rx_bytes, m_tx_bytes, address});

        return dynamic_cast<T&>(m_devices[address]);
    }

    template <typename T, std::enable_if_t<std::is_base_of<i2c_device, T>::value, bool> = true>
    [[nodiscard]] auto get(std::uint8_t address) -> T&
    {
        return dynamic_cast<T&>(m_devices[address]);
    }

    [[nodiscard]] auto is_open(std::uint8_t address) const -> bool;

    [[nodiscard]] auto close(std::uint8_t address) -> bool;

    [[nodiscard]] auto count_devices() const -> std::size_t;

    [[nodiscard]] auto rx_bytes() const -> std::size_t;
    [[nodiscard]] auto tx_bytes() const -> std::size_t;

    [[nodiscard]] auto get_devices() const -> const std::map<std::uint8_t, std::reference_wrapper<i2c_device>>&;

protected:
    std::string m_address { "/dev/i2c-1" };
    std::map<std::uint8_t, std::reference_wrapper<i2c_device>> m_devices {};

    std::size_t m_rx_bytes {};
    std::size_t m_tx_bytes {};

};
}

#endif // MUONPI_I2CINTERFACE_H
