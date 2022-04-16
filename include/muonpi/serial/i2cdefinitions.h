#ifndef HARDWARE_I2CDEFINITIONS_H
#define HARDWARE_I2CDEFINITIONS_H

#include "muonpi/concepts.h"

#include <algorithm>
#include <concepts>
#include <iterator>

namespace muonpi::serial {
template <typename T>
concept is_address_type =
    ((std::is_enum_v<T> && std::is_unsigned_v<std::underlying_type<T>>)
     || std::is_unsigned_v<T>)&&std::is_fundamental_v<T> && (!std::is_class_v<T>);

template <typename T>
concept is_value_type = std::integral<T> && (sizeof(T) <= sizeof(std::uint16_t))
                     && std::is_fundamental_v<T> && (!std::is_class_v<T>);
template <typename T>
concept address_iterable = requires(T t) {
    t.begin();
    t.end();
    { typename std::iterator_traits<decltype(t.begin())>::value_type() }
    ->is_address_type;
};

template <typename T>
concept value_iterable = requires(T t) {
    t.begin();
    t.end();
    { typename std::iterator_traits<decltype(t.begin())>::value_type() }
    ->is_value_type;
};
template <typename T>
concept address_iterator = requires(T) {
    { typename std::iterator_traits<T>::value_type() }
    ->is_address_type;
};

template <typename T>
concept value_iterator = requires(T) {
    { typename std::iterator_traits<T>::value_type() }
    ->is_value_type;
};
namespace detail {
template <typename T>
concept is_dev_type = requires(T t) {
    { T::addresses }
    ->address_iterable;
};
} // namespace detail

template <typename T>
concept is_register_type =
    is_value_type<typename T::value_type>&& is_address_type<typename T::address_type>&&
        std::is_class_v<T>&& is_address_type<decltype(T::address)>;

class i2c_device;
class i2c_bus;

template <typename T>
concept is_device_type =
    detail::is_dev_type<T>&&
        std::derived_from<T, i2c_device> && (std::begin(T::addresses) != std::end(T::addresses))
    && std::is_class_v<T>;

template <typename T, typename A = std::uint8_t, A ADDR = 0x00>
requires is_value_type<T>&& is_value_type<A>
    /**
     * @brief The i2c_register struct
     * Used to safely represent a i2c register address.
     */
    struct i2c_register {
    constexpr static A address {ADDR};

    [[nodiscard]] constexpr static auto base() -> i2c_register<T, A, ADDR> {
        return i2c_register<T, A, ADDR> {};
    }
};

template <typename T, typename A = std::uint8_t, A ADDR = 0x00>
requires is_value_type<T>&& is_value_type<A>
    /**
     * @brief The simple_register struct
     * Defines a register with a single value oftype value_type.
     */
    struct simple_register : public i2c_register<T, A, ADDR> {
    using value_type   = T;
    using address_type = A;

    /**
     * @brief get Get the raw value of the register.
     * @return The raw value of type value_type.
     */
    [[nodiscard]] constexpr virtual auto get() const noexcept -> value_type = 0;
};

template <typename T, std::uint8_t N, typename A = std::uint8_t, A ADDR = 0x00>
requires is_value_type<T>&& is_value_type<A>
    /**
     * @brief The multi_register struct
     * Defines a register with N values of type value_type
     */
    struct multi_register : public i2c_register<T, A, ADDR> {
    using value_type   = T;
    using address_type = A;

    /**
     * @brief get Get the raw value of the register
     * @return The raw array of data of type value_type.
     */
    [[nodiscard]] constexpr virtual auto get() const noexcept -> std::array<T, N> = 0;
};

namespace i2c_register_tag {
constexpr static std::uint8_t read {0b01};
constexpr static std::uint8_t write {0b10};
constexpr static std::uint8_t read_write {read | write};
} // namespace i2c_register_tag

template <typename T>
concept read_register =
    is_register_type<T> && ((T::register_tag & i2c_register_tag::read) == i2c_register_tag::read);

template <typename T>
concept write_register =
    is_register_type<T> && ((T::register_tag & i2c_register_tag::write) == i2c_register_tag::write);

template <typename T>
concept read_write_register =
    is_register_type<
        T> && ((T::register_tag & i2c_register_tag::read_write) == i2c_register_tag::read_write);

} // namespace muonpi::serial

#endif // HARDWARE_I2CDEFINITIONS_H
