#ifndef MUONPI_SERIAL_I2CBUS_H
#define MUONPI_SERIAL_I2CBUS_H

#include "muonpi/serial/i2cdevice.h"

#include <cinttypes>
#include <cstdio>
#include <fcntl.h> // open
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <sys/ioctl.h> // ioctl
#include <sys/time.h>  // for gettimeofday()
#include <type_traits>
#include <unistd.h>

namespace muonpi::serial {

/**
* @brief The i2c_bus class
* This class defines and abstracts an access interface to the hardware i2c bus (master mode) of the system.
*/
class i2c_bus {
public:
    friend class i2c_device;

    /**
    * @brief The general_call_t struct
    * This member struct of i2c_bus provides access to the i2c general call mechanism.
    * The general call is a special i2c device which issues commands at address 0x00.
    * Many devices listen to these 'broadcast' commands and take action
    */
    struct general_call_t {
    public:
        explicit general_call_t(i2c_bus* bus);

        /**
        * @brief issue a general call reset command
        * @return true, if the command could be issued successful
        */
        auto reset() -> bool;

        /**
        * @brief issue a general call wake-up command
        * @return true, if the command could be issued successful
        */
        auto wake_up() -> bool;

        /**
        * @brief issue a general call software-update command
        * @return true, if the command could be issued successful
        */
        auto software_update() -> bool;

    private:
        i2c_bus* m_bus { nullptr };
    } general_call { nullptr };

    /**
    * @brief constructor with specific device address path
    * @param address the system device address path of the i2c bus.
    * @note On *ix systems this is usually /dev/i2c-x
    */
    explicit i2c_bus(std::string address);

    /**
    * @brief default constructor without specific device address path
    * @note To not specify a device path at construction time of @link i2c_bus will be utilized for the
    * instantiation of sub-buses (e.g. from address translators). However, this functionality is not available
    * yet and will be implemented in the future.
    * @todo Still need to implement the sub-bus functionality, where the default constructor will be reasonably defined.
    */
    explicit i2c_bus();

    virtual ~i2c_bus();

    /**
    * @brief get the system address path of the bus
    * @return a string holding the system address of the bus
    */
    [[nodiscard]] auto address() const -> std::string;

    /**
    * @brief open a i2c device on the bus for access
    * @param address i2c device address in the range 0x01...0x7f
    * @return a reference to the device object
    * @note the template parameter T specifies the particular device which must be a descendant of @link i2c_device
    */
    template <typename T, std::enable_if_t<std::is_base_of<i2c_device, T>::value, bool> = true>
    [[nodiscard]] auto open(std::uint8_t address) -> T& {
        m_devices.emplace(address, std::make_shared<T>(*this, address));

        return this->get<T>(address);
    }

    /**
    * @brief get reference of existing i2c device with given address
    * @param address i2c device address in the range 0x01...0x7f
    * @return a reference to the device object
    * @note the template parameter T specifies the particular device which must be a descendant of @link i2c_device
    */
    template <typename T, std::enable_if_t<std::is_base_of<i2c_device, T>::value, bool> = true>
    [[nodiscard]] auto get(std::uint8_t address) -> T& {
        return dynamic_cast<T&>(*(m_devices[address].get()));
    }

    /**
    * @brief identify an i2c device of type T with given address
    * @param address i2c device address in the range 0x01...0x7f
    * @return result of the identity check. true, if the device of type T was responding on the specified address
    * @note the template parameter T specifies the particular device which must be a descendant of @link i2c_device
    */
    template <typename T, std::enable_if_t<std::is_base_of<i2c_device, T>::value, bool> = true>
    [[nodiscard]] auto identify_device(std::uint8_t address) -> bool;

    /**
    * @brief identify one or more i2c devices of type T
    * @param possible_addresses address range in which the devices will be searched for
    * @return a list of addresses at which devices of type T were responding
    * @note the template parameter T specifies the particular device which must be a descendant of @link i2c_device
    */
    template <typename T, std::enable_if_t<std::is_base_of<i2c_device, T>::value, bool> = true>
    [[nodiscard]] auto
    identify_devices(const std::set<std::uint8_t>& possible_addresses = std::set<std::uint8_t>())
        -> std::set<std::uint8_t>;

    /**
    * @brief check whether a device was opened for access
    * @param address i2c device address
    * @return true, if the device was opened for access
    * @note A positive return value does not imply that an actual device is physically present at the specified
    * address. The device was merely instantiated and opened for access. Yet, the result of bus transactions
    * are not reflected by this query.Use @link i2c_device#present to check for the physical presence of devices.
    */
    [[nodiscard]] auto is_open(std::uint8_t address) const -> bool;

    /**
    * @brief close a device which was previously opened for access
    * @param address i2c device address
    * @return true, if the bus could successfully release the device
    */
    auto close(std::uint8_t address) -> bool;

    /**
    * @brief the number of devices which are currently opened for access
    * @return number of devices open for access
    */
    [[nodiscard]] auto count_devices() const -> std::size_t;

    /**
    * @brief the total number of rx bytes transferred through the bus, i.e. read by the bus
    * @return total number of bytes read
    */
    [[nodiscard]] auto rx_bytes() const -> std::size_t;

    /**
    * @brief the total number of tx bytes transferred through the bus, i.e. written by the bus
    * @return total number of bytes written
    */
    [[nodiscard]] auto tx_bytes() const -> std::size_t;

    /**
    * @brief get list of all devices currently open for access
    * @return const reference of the device map
    */
    [[nodiscard]] auto get_devices() const -> const std::map<std::uint8_t, std::shared_ptr<i2c_device>>&;

protected:
    std::string                                         m_address {"/dev/i2c-1"};
    std::map<std::uint8_t, std::shared_ptr<i2c_device>> m_devices {};

    std::size_t m_rx_bytes {};
    std::size_t m_tx_bytes {};
};

template <typename T, std::enable_if_t<std::is_base_of<i2c_device, T>::value, bool>>
auto i2c_bus::identify_device(std::uint8_t address) -> bool {
    T dev {*this, address};
    if (!dev.is_open() || !dev.present()) {
        return false;
    }
    return dev.identify();
}

template <typename T, std::enable_if_t<std::is_base_of<i2c_device, T>::value, bool>>
auto i2c_bus::identify_devices(const std::set<std::uint8_t>& possible_addresses)
    -> std::set<std::uint8_t> {
    std::set<std::uint8_t> found_addresses {};

    for (const auto address : possible_addresses) {
        if (identify_device<T>(address)) {
            found_addresses.insert(address);
        }
    }
    return found_addresses;
}

} // namespace muonpi::serial

#endif // MUONPI_SERIAL_I2CBUS_H
