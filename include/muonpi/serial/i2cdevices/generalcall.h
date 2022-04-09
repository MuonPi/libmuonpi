#ifndef MUONPI_SERIAL_I2CDEVICES_GENERALCALL_H
#define MUONPI_SERIAL_I2CDEVICES_GENERALCALL_H

#include "muonpi/global.h"
#include "muonpi/serial/i2cdevice.h"

#include <array>

namespace muonpi::serial {

/**
 * @brief The general_call class
 * Sends commands to the bus on the address 0x00.
 * This is an address most devices would react to, so it can be used to send broadcast messages.
 */
class LIBMUONPI_PUBLIC general_call : public i2c_device {
public:
    constexpr static std::array<i2c_device::address_type, 1> addresses {0x00};

    general_call(traffic_t&, const std::string&, i2c_device::address_type) = delete;

    general_call(traffic_t& traffic, const std::string& path);

    /**
     * @brief reset Reset all devices on the bus
     * @return
     */
    auto reset() -> bool;

    /**
     * @brief wake_up Wake all devices up
     * @return
     */
    auto wake_up() -> bool;

    /**
     * @brief firmware_update Send a command to all devices so they listen to firmware updates.
     * @return
     */
    auto firmware_update() -> bool;
};

} // namespace muonpi::serial

#endif // MUONPI_SERIAL_I2CDEVICES_GENERALCALL_H
