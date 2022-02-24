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

/**
* @brief The i2c_device class
* This class defines basic i/o operations for a single i2c device.
* Basic data exchange methods are implemented, such as read and write of byte- and word-aligned data blocks.
*/
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

    /**
    * @brief constructor with device address
    * @param bus a reference to the i2c bus the device is attached to
    * @param address the i2c device address under which this object communicates at the bus
    */
    i2c_device(i2c_bus& bus, std::uint8_t address);
    explicit i2c_device(i2c_bus& bus);

    /**
    * @brief set the i2c address on the bus for this device
    * @param address the address under which the device will communicate on the bus
    */
    void               set_address(std::uint8_t address);

    /**
    * @brief get the i2c address on the bus for this device
    * @return the device's bus address
    */
    [[nodiscard]] auto address() const -> std::uint8_t {
        return m_address;
    }

    virtual ~i2c_device();

    /**
    * @brief check whether a device was opened for access
    * @return true, if the device was opened for access
    * @note A positive return value does not imply that an actual device is physically present.
    * The device was merely instantiated and opened for access.
    * Yet, the result of bus transactions is not reflected by this query.
    * Use @link i2c_device#present (if implemented) to check for the physical presence of a device.
    */
    [[nodiscard]] auto is_open() const -> bool;

    /**
    * @brief close a device which was previously opened for access
    * @return true, if the bus could successfully release the device
    */
    void               close() const;

    void                       read_capabilities() const;

    /**
    * @brief check for the presence of a device on the bus
    * @return true, if a device is found
    * @note: this method may be overriden in derived classes. If not, it tries to read a byte succesfully
    * from the device and returns the result of this read operation. This may not work for some devices,
    * so providing an individual version of this presence probing method is a good idea in these cases.
    */
    [[nodiscard]] virtual auto present() -> bool;

    /**
    * @brief check for the presence of a specific device on the bus
    * @return true, if the device could be identified
    * @note: this method should be overriden in derived classes, if there is a possibility to tell
    * the presence of a specific device from the bit battern read. If it is not reimplemented,
    * this method returns false by default.
    */
    [[nodiscard]] virtual auto identify() -> bool;

    [[nodiscard]] auto io_errors() const -> std::size_t;

    /**
    * @brief the total number of bytes read from the device so far
    * @return total number of bytes read
    */
    [[nodiscard]] auto rx_bytes() const -> std::size_t;

    /**
    * @brief the total number of bytes written to the device so far
    * @return total number of bytes written
    */
    [[nodiscard]] auto tx_bytes() const -> std::size_t;

    [[nodiscard]] auto flag_set(Flags flag) const -> bool;

    void               lock(bool locked = true);
    [[nodiscard]] auto locked() const -> bool;

    /**
    * @brief get the transfer duration of the last measured access
    * @return the duration of last transfer
    */
    [[deprecated]] [[nodiscard]] auto last_interval() const -> double;

    /**
    * @brief get the transfer duration of the last measured access
    * @return the duration of last transfer
    */
    [[nodiscard]] auto                last_access_duration() const -> std::chrono::microseconds;

    /**
    * @brief set the name of the i2c device object
    * @param name the name string
    */
    void               set_name(std::string name);

    /**
    * @brief get the name of the i2c device object
    * @return the name string
    */
    [[nodiscard]] auto name() const -> std::string;

    [[nodiscard]] auto addresses_hint() const -> const std::set<std::uint8_t>&;

    /**
    * @brief read an array of bytes from the device
    * @param buffer pointer to the buffer in which the data shall be placed
    * @param bytes the number of bytes to read
    * @return number of bytes actually read, or -1 on error
    */
    [[nodiscard]] auto read(std::uint8_t* buffer, std::size_t bytes = 1) -> int;

    [[nodiscard]] auto read(std::uint8_t reg, std::uint8_t bit_mask) -> std::uint16_t;

    [[nodiscard]] auto read(std::uint8_t reg, std::uint16_t bit_mask) -> std::uint32_t;

    /**
    * @brief read an array of bytes from sub-address of the device
    * @param reg sub-address (8bit register) from which the data should be read
    * @param buffer pointer to the buffer in which the data shall be placed
    * @param bytes the number of bytes to read
    * @return number of bytes actually read, or -1 on error
    */
    [[nodiscard]] auto read(std::uint8_t reg, std::uint8_t* buffer, std::size_t bytes = 1) -> int;

    /**
    * @brief read an array of words from sub-address of the device
    * @param reg sub-address (8bit register) from which the data should be read
    * @param buffer pointer to the buffer in which the data shall be placed
    * @param n_words the number of words to read
    * @return number of words actually read, or -1 on error
    */
    [[nodiscard]] auto read(std::uint8_t reg, std::uint16_t* buffer, std::size_t n_words = 1)
        -> int;

    /**
    * @brief write an array of bytes to the device
    * @param buffer pointer to the buffer with the data to be written
    * @param bytes the number of bytes to write
    * @return number of bytes actually written, or -1 on error
    */
    auto write(std::uint8_t* buffer, std::size_t bytes = 1) -> int;

    auto write(std::uint8_t reg, std::uint8_t bit_mask, std::uint8_t value) -> bool;

    auto write(std::uint8_t reg, std::uint8_t* buffer, std::size_t bytes = 1) -> int;

    /**
    * @brief write an array of words to sub-address of the device
    * @param reg sub-address (8bit register) to which the data should be written
    * @param buffer pointer to the buffer with the data to be written
    * @param length the number of words to write
    * @return number of words actually written, or -1 on error
    */
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
