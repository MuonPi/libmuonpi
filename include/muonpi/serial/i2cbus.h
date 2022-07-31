#ifndef HARDWARE_I2CBUS_H
#define HARDWARE_I2CBUS_H

#include "muonpi/serial/i2cdefinitions.h"
#include "muonpi/serial/i2cdevice.h"

#include <atomic>
#include <cinttypes>
#include <cstdio>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <sys/ioctl.h>
#include <type_traits>
#include <unistd.h>

namespace muonpi::serial {

class LIBMUONPI_PUBLIC i2c_bus {
public:
    /**
     * @brief i2c_bus
     * @param path the path to the i2c device. e.g. "/dev/i2c-1"
     */
    explicit i2c_bus(std::string path);

    i2c_bus();

    i2c_bus(const i2c_bus&) = delete;
    void operator=(const i2c_bus&) = delete;
    i2c_bus(i2c_bus&&)             = delete;
    void operator=(i2c_bus&&) = delete;

    virtual ~i2c_bus();

    /**
     * @brief path
     * @return The currently used path
     */
    [[nodiscard]] auto path() const -> std::string;

    template <i2c_device_type T>
    /**
     * @brief get Get a device on address.
     * If it is not yet open, opens the device.
     * @param address
     */
    [[nodiscard]] auto get(std::uint8_t address) -> T&;

    template <i2c_device_type T>
    /**
     * @brief get Get a device
     * If it is not yet open, opens the device.
     * Tries to automatically find the address of the device.
     */
    [[nodiscard]] auto get() -> T&;

    template <i2c_address_type A>
    /**
     * @brief identify_device Try to positively identify a device.
     * The exact method depends on the specific device, specifically if the manufacturer
     * provided a mechanism.
     * @param address
     * @return true if the device could be identified.
     */
    [[nodiscard]] auto identify_device(A address) -> bool;

    template <i2c_address_type A>
    /**
     * @brief is_open Check if a previously opened device is actually open.
     * @param address The address on the bus
     * @return true if the device is open
     */
    [[nodiscard]] auto is_open(A address) const -> bool;

    /**
     * @brief close Close a previously opened device.
     * @param address The address on the bus
     * @return true if the device was open before.
     */
    auto close(std::uint8_t address) -> bool;

    /**
     * @brief count_devices
     * @return The number of open devices.
     */
    [[nodiscard]] auto count_devices() const -> std::size_t;

    /**
     * @brief rx_bytes
     * @return The number of received bytes on the bus.
     */
    [[nodiscard]] auto rx_bytes() const -> std::size_t;

    /**
     * @brief rx_bytes
     * @return The number of transmitted bytes on the bus.
     */
    [[nodiscard]] auto tx_bytes() const -> std::size_t;

    /**
     * @brief get_devices
     * @return A map of all currently open devices
     */
    [[nodiscard]] auto get_devices() const
        -> const std::map<std::uint8_t, std::shared_ptr<i2c_device>>&;

private:
    std::string m_path {"/dev/i2c-1"}; //<! The path to the i2c bus device file
    std::map<std::uint8_t, std::shared_ptr<i2c_device>>
        m_devices {}; //<! Map of all opened devices on this bus

    i2c_device::traffic_t m_traffic_data {};
};

template <i2c_device_type T>
auto i2c_bus::get(std::uint8_t address) -> T& {
    if (m_devices.find(static_cast<std::uint8_t>(address)) == m_devices.end()) {
        m_devices.emplace(static_cast<std::uint8_t>(address),
                          std::make_shared<T>(m_traffic_data, path(), address));
    }

    return dynamic_cast<T&>(*(m_devices[static_cast<std::uint8_t>(address)].get()));
}

template <i2c_device_type T>
auto i2c_bus::get() -> T& {
    for (const auto& address : T::addresses) {
        if (m_devices.find(static_cast<std::uint8_t>(address)) != m_devices.end()) {
            return dynamic_cast<T&>(*(m_devices[static_cast<std::uint8_t>(address)].get()));
        }
    }
    auto device {std::make_shared<T>(m_traffic_data, path())};

    const auto address {device->address()};

    m_devices.emplace(address, std::move(device));

    return dynamic_cast<T&>(*(m_devices[static_cast<std::uint8_t>(address)].get()));
}

template <i2c_address_type A>
auto i2c_bus::identify_device(A address) -> bool {
    const auto iterator = m_devices.find(static_cast<std::uint8_t>(address));
    if (iterator == m_devices.end()) {
        return false;
    }

    auto& device {*(iterator->second)};
    if (!device.is_open() || !device.present()) {
        return false;
    }
    return device.identify();
}

template <i2c_address_type A>
auto i2c_bus::is_open(A address) const -> bool {
    const auto iterator = m_devices.find(static_cast<std::uint8_t>(address));
    if (iterator == m_devices.end()) {
        return false;
    }
    return (iterator->second)->is_open();
}

} // namespace muonpi::serial

#endif // HARDWARE_I2CBUS_H
