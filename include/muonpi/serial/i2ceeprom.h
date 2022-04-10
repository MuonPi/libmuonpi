#ifndef MUONPI_SERIAL_I2C_EEPROM_H
#define MUONPI_SERIAL_I2C_EEPROM_H

#include "muonpi/serial/i2cdefinitions.h"
#include "muonpi/serial/i2cdevice.h"

#include <thread>

namespace muonpi::serial {
namespace detail {
constexpr static std::uint16_t default_eeprom_length {256};
constexpr static std::uint16_t default_eeprom_pagesize {8};
} // namespace detail

/**
 * @brief The i2c_eeprom class.
 * Generic class template for i2c eeprom devices.
 * @param WORDS The total capacity of the eeprom in units of sizeof(value_type)
 * @param ADDR_T the address type. Usually should either be value_type or std::uint16_t.
 * @param PAGESIZE The page size in bytes which can be written in one chunk
 * @param VALUE_T the value type. Usually value_type.
 */
template <std::uint16_t WORDS  = detail::default_eeprom_length,
          typename ADDR_T      = std::uint8_t,
          std::size_t PAGESIZE = detail::default_eeprom_pagesize,
          typename VALUE_T     = std::uint8_t>
    requires((WORDS % PAGESIZE) == 0)
    && is_address_type<ADDR_T> class LIBMUONPI_PUBLIC i2c_eeprom : public i2c_device {
public:
    using address_type                            = ADDR_T;
    using value_type                              = VALUE_T;
    constexpr static std::size_t   address_length = sizeof(address_type);
    constexpr static std::uint16_t eeprom_length  = WORDS;
    constexpr static std::uint16_t page_size      = PAGESIZE;

    ~i2c_eeprom() override;

    /** Read multiple bytes starting from given address from EEPROM memory.
     * @param start_addr First register address to read from
     * @param num_bytes Number of bytes to read
     * @return std::nullopt in case of error, vector containing the read data otherwise.
     */
    [[nodiscard]] auto read(address_type start_addr, std::size_t num_bytes = 1)
        -> std::optional<std::vector<value_type>>;

    /** Write multiple bytes starting from given address into EEPROM memory.
     * @param addr First register address to write to
     * @param begin Iterator to first element to write.
     * @param end Iterator to last element to write
     * @return std::nullopt in case of failure. Number of bytes writte otherwise.
     */
    [[nodiscard]] auto write(address_type addr, value_iterator auto begin, value_iterator auto end)
        -> std::optional<int>;

    /** Write multiple bytes starting from given address into EEPROM memory.
     * @param addr First register address to write to.
     * @para values Iterable object containting the values to write.
     * @return std::nullopt in case of failure. Number of bytes writte otherwise.
     */
    [[nodiscard]] auto write(address_type addr, value_iterable auto values) -> std::optional<int>;

protected:
    /**
     * @brief i2c_eeprom
     * @param bus_traffic Traffic trackig object for the bus
     * @param address
     */
    i2c_eeprom(traffic_t& bus_traffic, const std::string& path, value_type address);

    i2c_eeprom(traffic_t& bus_traffic, const std::string& path, address_iterable auto addresses);

    static constexpr std::chrono::microseconds s_write_idle_time {5000};
};

/*********************
 * Implementation part
 *********************/
template <std::uint16_t WORDS, typename ADDR_T, std::size_t PAGESIZE, typename VALUE_T>
    requires((WORDS % PAGESIZE) == 0)
    && is_address_type<ADDR_T> i2c_eeprom<WORDS, ADDR_T, PAGESIZE, VALUE_T>::i2c_eeprom(
        traffic_t&            bus_traffic,
        const std::string&    path,
        address_iterable auto addresses)
    : i2c_device {bus_traffic, path, addresses} {}

template <std::uint16_t WORDS, typename ADDR_T, std::size_t PAGESIZE, typename VALUE_T>
    requires((WORDS % PAGESIZE) == 0)
    && is_address_type<ADDR_T> i2c_eeprom<WORDS, ADDR_T, PAGESIZE, VALUE_T>::~i2c_eeprom() =
    default;

template <std::uint16_t WORDS, typename ADDR_T, std::size_t PAGESIZE, typename VALUE_T>
    requires((WORDS % PAGESIZE) == 0)
    && is_address_type<ADDR_T> i2c_eeprom<WORDS, ADDR_T, PAGESIZE, VALUE_T>::i2c_eeprom(
        traffic_t&         bus_traffic,
        const std::string& path,
        value_type         address)
    : i2c_device {bus_traffic, path, address} {}

template <std::uint16_t WORDS, typename ADDR_T, std::size_t PAGESIZE, typename VALUE_T>
    requires((WORDS % PAGESIZE) == 0)
    && is_address_type<ADDR_T> auto i2c_eeprom<WORDS, ADDR_T, PAGESIZE, VALUE_T>::read(
           address_type start_addr,
           std::size_t  num_bytes) -> std::optional<std::vector<value_type>> {
    if (start_addr + num_bytes >= WORDS) {
        return std::nullopt;
    }

    std::size_t total_read {0};

    std::vector<value_type> page_data {};

    for (auto addr {start_addr}; addr < start_addr + num_bytes;) {
        const std::size_t bytes {std::min(PAGESIZE - addr % PAGESIZE, num_bytes - total_read)};

        if (!i2c_device::write(addr)) {
            return std::nullopt;
        }

        auto status {i2c_device::read<value_type>(bytes)};

        if (!status) {
            return std::nullopt;
        }

        auto data {status.value()};
        std::copy(std::begin(data), std::end(data), std::back_inserter(page_data));

        addr += data.size();
        total_read += data.size();
    }
    return page_data;
}

template <std::uint16_t WORDS, typename ADDR_T, std::size_t PAGESIZE, typename VALUE_T>
    requires((WORDS % PAGESIZE) == 0)
    && is_address_type<ADDR_T> auto i2c_eeprom<WORDS, ADDR_T, PAGESIZE, VALUE_T>::write(
           address_type        start_addr,
           value_iterator auto begin,
           value_iterator auto end) -> std::optional<int> {
    int total_written {0};

    const std::size_t num_bytes {std::distance(begin, end)};

    if (start_addr + (num_bytes) >= eeprom_length) {
        return std::nullopt;
    }

    auto current = begin;

    for (auto addr {start_addr}; addr < start_addr + num_bytes;) {
        const std::size_t bytes {std::min(page_size - addr % page_size, num_bytes - total_written)};

        std::vector<value_type> write_data {};

        if constexpr (address_length == 1) {
            write_data.emplace_back(static_cast<value_type>(addr));
        } else {
            write_data.emplace_back(static_cast<value_type>(
                addr
                >> 8U)); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            write_data.emplace_back(static_cast<value_type>(addr));
        }

        std::copy(current, current + bytes, std::back_inserter(write_data));

        const auto written {write(write_data.begin(), write_data.end())};

        if (written.value_or(0) != (bytes + address_length)) {
            return std::nullopt;
        }

        addr += bytes;
        total_written += bytes;
        current += bytes;

        std::this_thread::sleep_for(s_write_idle_time);
    }
    return total_written;
}

template <std::uint16_t WORDS, typename ADDR_T, std::size_t PAGESIZE, typename VALUE_T>
    requires((WORDS % PAGESIZE) == 0)
    && is_address_type<ADDR_T> auto i2c_eeprom<WORDS, ADDR_T, PAGESIZE, VALUE_T>::write(
           address_type        start_addr,
           value_iterable auto values) -> std::optional<int> {
    return write(start_addr, std::begin(values), std::end(values));
}

} // namespace muonpi::serial

#endif // MUONPI_SERIAL_I2C_EEPROM_H
