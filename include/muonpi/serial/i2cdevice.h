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

/// definition of invalid I2C address
constexpr std::uint8_t InvalidI2cAddress {0xff};

template <class T>
/**
 * @brief helper struct for providing static methods to all classes derived from i2c_device
 * @note inheriting this struct facilitates descendants of i2c_device with all methods
 * from this struct utilizing the Curiously Recursive Template Pattern (CRTP) mechanism
 */
struct static_device_base {
    /**
     * @brief get a list of possible addresses at which the device can be found
     * @return a set of potential i2c bus addresses
     * @note The default addresses list is a hint consisting of a collection of
     * potential addresses at which the device may respond and is defined by
     * the hardware-wise assignable addresses
     * from manufacturer side.
     * If this list is non-empty, one can safely assume that the device address
     * will be among these addresses.
     */
    [[nodiscard]] static auto default_addresses() -> std::set<std::uint8_t>;

    /**
     * @brief get the name of this i2c device
     * @return the name string
     */
    [[nodiscard]] static auto device_name() -> std::string;
};

/**
 * @brief The i2c_device class
 * This class defines basic i/o operations for a single i2c device.
 * Basic data exchange methods are implemented, such as read and write of byte- and word-aligned
 * data blocks.
 */
class i2c_device {
public:
    /** @brief Flags enum defining the possible operation states of the device
     */
    enum class Flags : std::uint8_t
    {
        None        = 0,    //!< undefined, uninitialized or unknown state
        Normal      = 0x01, //!< normal operational state
        Force       = 0x02, //!< device is operational, but has conflicting access to another driver
        Unreachable = 0x04, //!< device does not respond
        Failed      = 0x08, //!< system interface could not open a channel to the device
        Locked      = 0x10  //!< device is operational, but access is inhibited
    };

    i2c_device() = delete;

    /**
     * @brief constructor with device address
     * @param bus a reference to the i2c bus the device is attached to
     * @param address optional i2c device address under which this object
     * communicates on the bus
     * @note if the device address is not specified it has to be explicitely set after
     * construction with @link #set_address set_address @endlink
     */
    i2c_device(i2c_bus& bus, std::uint8_t address = 0xff);

    /**
     * @brief set the i2c address on the bus for this device
     * @param address the address under which the device will communicate on the bus
     */
    void set_address(std::uint8_t address);

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
     * Use method @link i2c_device#present present() @endlink (if implemented) to check for the
     * physical presence of a device.
     */
    [[nodiscard]] auto is_open() const -> bool;

    /**
     * @brief close a device which was previously opened for access
     * @return true, if the bus could successfully release the device
     */
    void close() const;

    void read_capabilities() const;

    /**
     * @brief check for the presence of a device on the bus
     * @return true, if a device is found
     * @note this method may be overriden in derived classes. If not, it tries to read a byte
     * succesfully from the device and returns the result of this read operation. This may not work
     * for some devices, so providing an individual version of this presence probing method is a
     * good idea in these cases.
     */
    [[nodiscard]] virtual auto present() -> bool;

    /**
     * @brief check for the presence of a specific device on the bus
     * @return true, if the device could be identified
     * @note this method should be overriden in derived classes, if there is a possibility to tell
     * the presence of a specific device from the bit battern read. If it is not reimplemented,
     * this method returns false by default.
     */
    [[nodiscard]] virtual auto identify() -> bool;

    /**
     * @brief get the number of interface errors that occurred up to now
     * @return total number of interface errors so far
     * @note unsuccessfull accesses to the i2c bus functionality, e.g. read and write functions
     * that return with an error are accounted as interface errors.
     */
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

    /**
     * @brief lock the current device and prevent further transactions
     * @param locked the lock condition to be set
     * @note locking a device means that the device is still considered to be attached to the
     * i2c_bus but any communication with the device will be prevented
     */
    void lock(bool locked = true);

    /**
     * @brief get the lock status of the device
     * @return the lock condition currently set
     * @note locking a device means that the device is still considered to be attached to the
     * i2c_bus but any communication with the device will be prevented
     */
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
    [[nodiscard]] auto last_access_duration() const -> std::chrono::microseconds;

    /**
     * @brief set the name of the i2c device object
     * @param name the name string
     */
    void set_name(std::string name);

    /**
     * @brief get the name of the i2c device object
     * @return the name string
     */
    [[nodiscard]] auto name() const -> std::string;

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
     * @param reg sub-address (8-bit register) from which the data should be read
     * @param buffer pointer to the buffer in which the data shall be placed
     * @param bytes the number of bytes to read
     * @return number of bytes actually read, or -1 on error
     */
    [[nodiscard]] auto read(std::uint8_t reg, std::uint8_t* buffer, std::size_t bytes = 1) -> int;

    /**
     * @brief read an array of words from sub-address of the device
     * @param reg sub-address (8-bit register) from which the data should be read
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

    /**
     * @brief write an array of bytes to sub-address of the device
     * @param reg sub-address (8-bit register) to which the data should be written
     * @param buffer pointer to the buffer with the data to be written
     * @param bytes the number of bytes to write
     * @return number of bytes actually written, or -1 on error
     */
    auto write(std::uint8_t reg, std::uint8_t* buffer, std::size_t bytes = 1) -> int;

    /**
     * @brief write an array of words to sub-address of the device
     * @param reg sub-address (8-bit register) to which the data should be written
     * @param buffer pointer to the buffer with the data to be written
     * @param length the number of words to write
     * @return number of words actually written, or -1 on error
     */
    auto write(std::uint8_t reg, const std::uint16_t* buffer, std::size_t length = 1) -> int;

    /**
     * @brief get a list of possible addresses at which the device can be found
     * @return a set of potential i2c bus addresses
     * @note The addresses hint list is a collection of
     * potential addresses at which the device may respond and is defined by
     * the hardware-wise assignable addresses
     * from manufacturer side.
     * If this list is non-empty, one can safely assume that the device address
     * will be among these addresses.
     */
    [[nodiscard]] auto addresses_hint() const -> std::set<std::uint8_t> {
        return m_addresses_hint;
    }

protected:
    void set_flag(Flags flag);
    void unset_flag(Flags flag);

    /**
     * @brief start the timer to measure time intervals
     * @note a valid measurement of a time interval is only taken with a successive call
     * to @link stop_timer stop_timer() @endlink. The result of the measurement can then
     * be obtained through the @link last_access_duration last_access_duration()
     * @endlink method.
     */
    void start_timer();

    /**
     * @brief stop the timer for measuring a time interval
     * @note a valid measurement of a time interval is only taken with a preceding call
     * to @link start_timer start_timer() @endlink. The result of the measurement can
     * be obtained through the @link last_access_duration last_access_duration()
     * @endlink method.
     */
    void stop_timer();

    /**
     * @brief set up and start the timer to measure time intervals
     * @return a scope_guard object which stops the timer on destruction
     * @note Use this method to measure time intervals (e.g. access or readout intervals)
     * without explicitely calling the @link stop_timer stop_timer() @endlink method.
     * The returned @link scope_guard scope_guard @endlink object takes care for
     * stopping the timer as soon as it goes out of scope. The result of the measurement
     * can be obtained through the @link last_access_duration last_access_duration()
     * @endlink method.
     */
    [[nodiscard]] auto setup_timer() -> scope_guard;

    /**
     * @brief set the list of possible addresses at which the device can be found
     * @param address_list a std::set of potential i2c bus addresses
     * @note The addresses hint list is a collection of potential addresses at which
     * the device may respond and is defined by the hardware-wise assignable addresses
     * from the manufacturer side.
     * Use this method to define the hardware-addressable range as indicated in
     * the device's datasheet
     */
    void set_addresses_hint(std::set<std::uint8_t> address_list);

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

    std::set<std::uint8_t> m_addresses_hint {};
};

template <class T>
auto static_device_base<T>::default_addresses() -> std::set<std::uint8_t> {
    i2c_bus bus {};
    T       dev(bus);
    return dev.addresses_hint();
}

template <class T>
auto static_device_base<T>::device_name() -> std::string {
    i2c_bus bus {};
    T       dev(bus);
    return dev.name();
}

} // namespace muonpi::serial

#endif // MUONPI_SERIAL_I2CDEVICE_H
