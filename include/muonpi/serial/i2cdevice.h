#ifndef HARDWARE_I2CDEVICE_H
#define HARDWARE_I2CDEVICE_H

#include "muonpi/log.h"
#include "muonpi/serial/i2cdefinitions.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cinttypes>
#include <fcntl.h>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <sys/ioctl.h>
#include <type_traits>
#include <unistd.h>
#include <vector>

namespace muonpi::serial {

class LIBMUONPI_PUBLIC i2c_device {
public:
    using address_type = std::uint8_t;

    struct traffic_t {
        std::atomic<std::size_t> rx_bytes {}; //<! Total number of received bytes
        std::atomic<std::size_t> tx_bytes {}; //<! Total number of transmitted bytes
    };

    enum class Flags : std::uint8_t
    {
        None    = 0,
        Error   = 0b10000000,
        Warning = 0b01000000,
        Locked  = 0b00100000,

        Normal      = 0x01,
        Force       = Warning | 0x02U,
        Unreachable = Error | 0x04U,
        Failed      = Error | 0x08U
    };

    i2c_device(const i2c_device&) = delete;
    void operator=(const i2c_device&) = delete;
    i2c_device(i2c_device&&)          = delete;
    void operator=(i2c_device&&) = delete;

    virtual ~i2c_device();

    /**
     * @brief address Get the used i2c address.
     * @return The i2c address
     */
    [[nodiscard]] auto address() const -> address_type;

    /**
     * @brief is_open Check if the file descriptor is setup.
     * @return true if the file descriptor is connected.
     */
    [[nodiscard]] auto is_open() const -> bool;

    /**
     * @brief close Closes the file descriptor
     */
    void close();

    /**
     * @brief read_capabilities Read the capabilities of the i2c device.
     */
    void read_capabilities() const;

    /**
     * @brief present Checks whether the device is present.
     * @return true if present
     */
    [[nodiscard]] virtual auto present() -> bool;

    /**
     * @brief identify Attempts to positively identify the i2c device.
     * This may or may not be possible depending on the device manufacturer.
     * @return true if the device was successfully identified.
     */
    [[nodiscard]] virtual auto identify() -> bool;

    /**
     * @brief io_errors
     * @return The number of io errors in the device.
     */
    [[nodiscard]] auto io_errors() const -> std::size_t;

    /**
     * @brief rx_bytes
     * @return The number of received bytes.
     */
    [[nodiscard]] auto rx_bytes() const -> std::size_t;

    /**
     * @brief tx_bytes
     * @return The number of transmitted bytes.
     */
    [[nodiscard]] auto tx_bytes() const -> std::size_t;

    /**
     * @brief flag_set Checks whether a flag is set.
     * @param flag
     * @return true if the flag is set.
     */
    [[nodiscard]] auto flag_set(Flags flag) const -> bool;

    /**
     * @brief lock Set the locked flag of the device.
     * @param locked
     */
    void lock(bool locked = true);

    /**
     * @brief locked Get the locked flag of the device.
     * @return
     */
    [[nodiscard]] auto locked() const -> bool;

    /**
     * @brief writable
     * @return True if the device is open and not locked.
     */
    [[nodiscard]] auto writable() const -> bool;

    /**
     * @brief set_name Set an identifier for the device.
     * @param name
     */
    void set_name(std::string name);

    /**
     * @brief name Get the currently set identifier.
     * @return
     */
    [[nodiscard]] auto name() const -> std::string;

    template <i2c_value_type T>

    /**
     * @brief read Read a single byte or word from the device.
     * @param reg The register from which to read.
     * @return The read value.
     * nullopt in case of failure.
     */
    [[nodiscard]] auto read() -> std::optional<T>;

    template <read_register R>
    /**
     * @brief read Read an entire register from the device.
     * Takes a register with just one value.
     * @return The read value.
     * nullopt in case of failure.
     */
    [[nodiscard]] auto read() -> std::optional<R> requires (R::register_length == 1);

    template <read_register R>
    /**
     * @brief read Read an entire register from the device.
     * Takes registers with an array of values.
     * @return The read value.
     * nullopt in case of failure.
     */
    [[nodiscard]] auto read() -> std::optional<R> requires (R::register_length > 1);

    template <read_register R, std::uint8_t N>
    /**
     * @brief read Read a number of bytes or words from a register.
     * @return an array with the read data.
     * nullopt in case of failure.
     */
    [[nodiscard]] auto read()
        -> std::optional<std::array<typename R::value_type, N>> requires (N > 0);

    template <i2c_value_type T, std::uint8_t N>
    /**
     * @brief read Read a number of bytes or words from the device.
     * @param length Number of bytes or words to read.
     * @return a vector with the read data.
     * nullopt in case of failure.
     */
    [[nodiscard]] auto read() -> std::optional<std::array<T, N>> requires (N > 0);

    template <i2c_value_type T>

    /**
     * @brief write Write a single byte or word to the device.
     * @param value the value to write.
     * @param reg The register to write to.
     * @return True on success.
     */
    [[nodiscard]] auto write(T value) -> bool;

    template <write_register R>

    /**
     * @brief write Write a single byte or word to a register.
     * @param reg The register to write
     * @return True on success.
     */
    [[nodiscard]] auto write(R reg) -> bool;

    /**
     * @brief write Write either a byte or 16 bit word to the i2c device.
     * @param begin Iterator to the beginning of the memory space to write
     * @param end Iterator to the end of the memory space to write
     * @return The number of bytes or words written.
     * The number returned is `nbytes / sizeof(InputT)`
     * where InputT is the underlying type of the iterable object.
     * nullopt in case of failure.
     */
    [[nodiscard]] auto write(value_iterator auto begin, value_iterator auto end)
        -> std::optional<int>;

    /**
     * @brief write Write either a byte or 16 bit word to the i2c device.
     * @param values The iterable object containing the values to write
     * @return The number of bytes or words written.
     * The number returned is `nbytes / sizeof(InputT)`
     * where InputT is the underlying type of the iterable object.
     * nullopt in case of failure.
     */
    [[nodiscard]] auto write(value_iterable auto values) -> std::optional<int>;

    template <write_register R>
    /**
     * @brief write Write either a byte or 16 bit word to a register of the i2c device.
     * @param begin Iterator to the beginning of the memory space to write
     * @param end Iterator to the end of the memory space to write
     * @param reg The register address to write to. default = 0
     * @return The number of bytes or words written.
     * The number returned is `nbytes / sizeof(InputT)`
     * where InputT is the underlying type of the iterable object.
     * nullopt in case of failure.
     */
    [[nodiscard]] auto write(iterator<typename R::value_type> auto begin,
                             iterator<typename R::value_type> auto end) -> std::optional<int>;

    template <write_register R>
    /**
     * @brief write Write either a byte or 16 bit word to a register of the i2c device.
     * @param begin Iterator to the beginning of the memory space to write
     * @param end Iterator to the end of the memory space to write
     * @param reg The register address to write to. default = 0
     * @return The number of bytes or words written.
     * The number returned is `nbytes / sizeof(InputT)`
     * where InputT is the underlying type of the iterable object.
     * nullopt in case of failure.
     */
    [[nodiscard]] auto write(iterable<typename R::value_type> auto values) -> std::optional<int>;

    /**
     * @brief add_tx_bytes Increase the transmitted byte counter
     * @param bytes
     */
    void add_tx_bytes(std::size_t bytes);

    /**
     * @brief add_tx_bytes Increase the received byte counter
     * @param bytes
     */
    void add_rx_bytes(std::size_t bytes);

protected:
    /**
     * @brief i2c_device
     * @param bus_traffic Traffic trackig object for the bus
     * @param path the path of the I2C device
     * @param address
     */
    i2c_device(traffic_t& bus_traffic, const std::string& path, address_type address);

    /**
     * @brief i2c_device Attempt automatic setup of the device.
     * @param bus_traffic Traffic trackig object for the bus
     * @param path the path of the I2C device
     * @param addresses iterable object containting all valid addresses
     */
    i2c_device(traffic_t&                   bus_traffic,
               const std::string&           path,
               const address_iterable auto& addresses);
    /**
     * @brief set_flag Set a status flag
     * @param flag
     */
    void set_flag(Flags flag);
    /**
     * @brief unset_flag Unset a status flag
     * @param flag
     */
    void unset_flag(Flags flag);

    /**
     * @brief auto_setup attempt to automatically find the used address of the device.
     * @return False if the address could be found and setup.
     */
    [[nodiscard]] auto auto_setup(const address_iterable auto& addresses) -> bool;

    /**
     * @brief set_address Set the address of the i2c device.
     * @param address
     * @return False in case of failure.
     */
    [[nodiscard]] auto set_address(address_type address) -> bool;

private:
    traffic_t  m_traffic {};
    traffic_t& m_bus_traffic; //<! Reference to the managing i2c_bus object.

    address_type m_address {}; //<! The address to use for the i2c_device.

    int  m_handle {};      //<! The file descriptor for the open connection
    bool m_locked {false}; //<! The locked state of the device. If this is true, nothing will be
                           // read or written.

    std::size_t m_io_errors {}; //<! The number of experienced IO errors

    std::string  m_name {"I2C device"}; //<! A descriptive name for the device
    std::uint8_t m_flags {};            //<! Flags which represent the current status of the device

    template <typename InputIt,
              typename OutputIt,
              typename InputT  = typename std::iterator_traits<InputIt>::value_type,
              typename OutputT = typename std::iterator_traits<OutputIt>::value_type>

    void copy_from(InputIt s_begin, InputIt s_end, OutputIt d_begin) requires is_integral_size<
        InputT,
        OutputT>&& is_integral_size<OutputT, InputT>&& is_integral_size<OutputT, std::uint8_t>;

    template <typename InputIt,
              typename OutputIt,
              typename InputT  = typename std::iterator_traits<InputIt>::value_type,
              typename OutputT = typename std::iterator_traits<OutputIt>::value_type>
    void copy_from(InputIt s_begin, InputIt s_end, OutputIt d_begin) requires is_integral_size<
        InputT,
        std::uint16_t>&& is_integral_size<OutputT, std::uint8_t>;

    template <typename InputIt,
              typename OutputIt,
              typename InputT  = typename std::iterator_traits<InputIt>::value_type,
              typename OutputT = typename std::iterator_traits<OutputIt>::value_type>
    void copy_from(InputIt s_begin, InputIt s_end, OutputIt d_begin) requires is_integral_size<
        InputT,
        std::uint8_t>&& is_integral_size<OutputT, std::uint16_t>;
};

/////////////////////////////////
////// Implementation part //////
/////////////////////////////////

i2c_device::i2c_device(traffic_t&                   bus_traffic,
                       const std::string&           path,
                       const address_iterable auto& addresses)
    : m_bus_traffic {bus_traffic}
    , m_handle {open(path.c_str(), O_RDWR)} // NOLINT: cppcoreguidelines-pro-type-vararg
{
    if (!auto_setup(addresses)) {
        set_flag(Flags::Failed);
        throw std::runtime_error {("Could not auto initialise I2C device '" + path + "'").c_str()};
    }
}

auto i2c_device::auto_setup(const address_iterable auto& addresses) -> bool {
    return std::any_of(std::begin(addresses),
                       std::end(addresses),
                       [&](const auto& address) -> bool { return set_address(address); });
}

template <typename InputIt, typename OutputIt, typename InputT, typename OutputT>
void i2c_device::
    copy_from(InputIt s_begin, InputIt s_end, OutputIt d_begin) requires is_integral_size<
        InputT,
        OutputT>&& is_integral_size<OutputT, InputT>&& is_integral_size<OutputT, std::uint8_t> {
    while (s_begin != s_end) {
        *(d_begin++) = *(s_begin++);
    }
}

template <typename InputIt, typename OutputIt, typename InputT, typename OutputT>
void i2c_device::copy_from(
    InputIt s_begin,
    InputIt s_end,
    OutputIt
        d_begin) requires is_integral_size<InputT, std::uint16_t>&& is_integral_size<OutputT,
                                                                                     std::uint8_t> {
    while (s_begin != s_end) {
        *(d_begin++) =
            (*s_begin)
            >> 8U; // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        *(d_begin++) = (*s_begin++);
    }
}

template <typename InputIt, typename OutputIt, typename InputT, typename OutputT>
void i2c_device::copy_from(
    InputIt s_begin,
    InputIt s_end,
    OutputIt
        d_begin) requires is_integral_size<InputT, std::uint8_t>&& is_integral_size<OutputT,
                                                                                    std::uint16_t> {
    while (s_begin != s_end) {
        *(d_begin) = static_cast<OutputT>(*s_begin++)
                  << 8U; // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        *(d_begin++) |= (*s_begin++);
    }
}

template <i2c_value_type T>
auto i2c_device::read() -> std::optional<T> {
    if (!writable()) {
        return std::nullopt;
    }

    constexpr auto bytes {sizeof(T)};

    std::array<std::uint8_t, bytes> buffer { 0 };

    const auto nread = ::read(m_handle, buffer.data(), bytes);
    if (nread == bytes) {
        add_rx_bytes(nread);
        unset_flag(Flags::Unreachable);
    } else {
        set_flag(Flags::Unreachable);
        return std::nullopt;
    }

    if constexpr (bytes == 1) {
        return static_cast<T>(buffer[0]);
    }

    // casting multiple times due to integral promotion to `int`:
    // https://en.cppreference.com/w/cpp/language/implicit_conversion#Integral_promotion
    return static_cast<T>(
        (static_cast<T>(
            static_cast<T>(buffer[0])
            << 8U)) // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        | static_cast<T>(buffer[1]));
}

template <read_register R>
auto i2c_device::read() -> std::optional<R> requires (R::register_length == 1) {
    if (!write(R::address)) {
        return std::nullopt;
    }

    const auto result {read<typename R::value_type>()};

    if (!result) {
        return std::nullopt;
    }

    return R {result.value()};
}

template <read_register R>
auto i2c_device::read() -> std::optional<R> requires (R::register_length > 1) {
    if (!write(R::address)) {
        return std::nullopt;
    }

    const auto result { read<typename R::value_type, R::register_length>() };

    if (!result) {
        return std::nullopt;
    }

    return R {result.value()};
}

template <i2c_value_type T, std::uint8_t N>
auto i2c_device::read() -> std::optional<std::array<T, N>> requires (N > 0) {
    if (!writable()) {
        return std::nullopt;
    }

    constexpr auto factor {sizeof(T)};

    constexpr auto bytes { N * factor };

    std::array<std::uint8_t, bytes> buffer { 0 };

    const auto nread = ::read(m_handle, buffer.data(), bytes);
    add_rx_bytes(nread);
    if (nread <= 0) {
        set_flag(Flags::Unreachable);
        return std::nullopt;
    }
    unset_flag(Flags::Unreachable);
    if (nread % factor != 0) {
        log::warning("i2c") << "incomplete read. Expected multiple of " << factor
                            << " number of bytes, got " << nread;
        set_flag(Flags::Warning);
        return std::nullopt;
    }
    if (nread < bytes) {
        log::warning("i2c") << "incomplete read. Expected " << bytes
                            << " number of bytes, got " << nread;
        set_flag(Flags::Warning);
        return std::nullopt;
    }
    std::array<T, N> output { 0 };

    copy_from(std::begin(buffer), std::begin(buffer) + nread, std::begin(output));

    return output;
}

template <read_register R, std::uint8_t N>
auto i2c_device::read() -> std::optional<std::array<typename R::value_type, N>> requires (N > 0) {
    if (!write(R::address)) {
        return std::nullopt;
    }

    return read<R::value_type, N>();
}

template <i2c_value_type T>
auto i2c_device::write(T value) -> bool {
    if (!writable()) {
        return false;
    }

    std::array<std::uint8_t, sizeof(T)> data {};

    if constexpr (sizeof(T) == sizeof(std::uint8_t)) {
        data[0] = static_cast<std::uint8_t>(value);
    } else {
        data[0] = static_cast<std::uint8_t>(
            value
            >> 8U); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        data[1] = static_cast<std::uint8_t>(value);
    }

    const auto nwritten = ::write(m_handle, data.data(), sizeof(T));
    if (static_cast<std::size_t>(nwritten) == data.size()) {
        add_tx_bytes(nwritten);
        unset_flag(Flags::Unreachable);
        return true;
    }
    set_flag(Flags::Unreachable);
    return false;
}

template <std::integral T, iterator<std::uint8_t> IT>
/**
 * @brief Set a value of either one or two byte size at an byte iterator position.
 * @param value The value to set
 * @param it The iterator to which to add the value
 * @returns The next iterator after the inserted value.
 */
[[nodiscard]] constexpr auto add_value(T value, IT it) -> IT requires (sizeof(T) <= 2) {
    if constexpr (sizeof(value) > 1) {
        *it = static_cast<std::uint8_t>(value >> 8U);
        ++it;
    }
    *it = static_cast<std::uint8_t>(value);
    ++it;
    return it;
}

template <write_register R>
auto i2c_device::write(R reg) -> bool {
    if (!writable()) {
        return false;
    }

    constexpr std::uint8_t bytes {(R::address == 0?0:1) + R::register_length*sizeof(typename R::value_type)};
    std::array<std::uint8_t, bytes> data {};
    auto begin { std::begin(data) };
    if constexpr (R::address != 0) {
        begin = add_value(R::address, begin);
    }

    const auto value {reg.get()};
    if constexpr (R::register_length > 1) {
        copy_from(std::begin(value), std::end(value), begin);
    } else {
        begin = add_value(value, begin);
    }

    const auto nwritten = ::write(m_handle, data.data(), data.size());
    if (static_cast<std::size_t>(nwritten) == data.size()) {
        add_tx_bytes(nwritten);
        unset_flag(Flags::Unreachable);
        return true;
    }
    set_flag(Flags::Unreachable);
    return false;
}

auto i2c_device::write(value_iterator auto begin, value_iterator auto end) -> std::optional<int> {
    if (!writable()) {
        log::error("i2c") << "device not writeable.";
        return std::nullopt;
    }

    constexpr auto factor {sizeof(decltype(*begin))};

    const auto distance = std::distance(begin, end) * factor;
    if (distance <= 0) {
        log::error("i2c") << "No data to write.";
        return std::nullopt;
    }

    std::vector<std::uint8_t> data(distance, 0);

    copy_from(begin, end, std::begin(data));

    const int nwritten = ::write(m_handle, data.data(), distance);
    if (nwritten > 0) {
        add_tx_bytes(nwritten);
        unset_flag(Flags::Unreachable);
    } else {
        set_flag(Flags::Unreachable);
        log::error("i2c") << "Unreachable.";
        return std::nullopt;
    }
    return nwritten / factor;
}

auto i2c_device::write(value_iterable auto values) -> std::optional<int> {
    return write(std::begin(values), std::end(values));
}

template <write_register R>
auto i2c_device::write(iterator<typename R::value_type> auto begin,
                       iterator<typename R::value_type> auto end) -> std::optional<int> {
    if (!writable()) {
        log::error("i2c") << "device not writeable.";
        return std::nullopt;
    }

    constexpr auto factor {sizeof(delctype(*begin))};
    constexpr auto reg_size {sizeof(typename R::address_type)};

    const auto distance = std::distance(begin, end) * factor + reg_size;
    if (distance <= reg_size) {
        log::error("i2c") << "No data to write.";
        return std::nullopt;
    }

    std::vector<std::uint8_t> data(distance, 0);
    auto back { std::begin(data) };
    back = add_value(R::address, back);

    copy_from(begin, end, back);

    const int nwritten = ::write(m_handle, data.data(), distance);
    if (nwritten > 0) {
        add_tx_bytes(nwritten);
        unset_flag(Flags::Unreachable);
    } else {
        set_flag(Flags::Unreachable);
        log::error("i2c") << "Unreachable.";
        return std::nullopt;
    }
    return (nwritten - reg_size) / factor;
}

template <write_register R>
auto i2c_device::write(iterable<typename R::value_type> auto values) -> std::optional<int> {
    return write<R>(std::begin(values), std::end(values));
}

} // namespace muonpi::serial

#endif // HARDWARE_I2CDEVICE_H
