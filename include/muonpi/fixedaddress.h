#ifndef MUONPI_FIXEDADDRESS_H
#define MUONPI_FIXEDADDRESS_H

#include <array>
#include <concepts>
#include <cstdint>
#include <iterator>
namespace muonpi {

template <std::integral T = std::uint8_t>
/**
 * @brief The fixed_address class
 */
class fixed_address {
public:
    /**
     * @brief The iterator class
     */
    class iterator {
    public:
        using value_type        = T;
        using difference_type   = T;
        using pointer           = value_type*;
        using reference         = value_type&;
        using iterator_category = std::forward_iterator_tag;

        /**
         * @brief iterator Create an iterator
         * @param current The current value
         */
        explicit constexpr iterator(value_type current) noexcept;

        /**
         * @brief iterator Copy constructor
         * @param other
         */
        constexpr iterator(const iterator& other) noexcept;

        /**
         * @brief operator = copy assignment
         * @param other
         */
        constexpr auto operator=(const iterator& other) noexcept -> iterator&;

        /**
         * @brief iterator Move constructor
         * @param other
         */
        constexpr iterator(iterator&& other) noexcept;

        /**
         * @brief operator = Move assignment
         * @param other
         */
        constexpr auto operator=(iterator&& other) noexcept -> iterator&;

        /**
         * @brief ~iterator
         */
        constexpr ~iterator() noexcept;

        /**
         * @brief operator * Dereference the iterator
         * @return The current value
         */
        [[nodiscard]] constexpr auto operator*() const noexcept -> value_type;
        /**
         * @brief operator ++ Prefix increment the iterator
         */
        constexpr auto operator++() noexcept -> iterator&;
        /**
         * @brief operator ++ Postfix increment the iterator
         * @return
         */
        constexpr auto operator++(int) noexcept -> iterator;

        /**
         * @brief operator !=
         * @param other
         * @return
         */
        [[nodiscard]] constexpr auto operator!=(const iterator& other) const noexcept -> bool;
        /**
         * @brief operator ==
         * @param other
         * @return
         */
        [[nodiscard]] constexpr auto operator==(const iterator& other) const noexcept -> bool;

    private:
        value_type                m_current {};
    };

    /**
     * @brief fixed_address Creae a fixed_address object
     * @param address The address
     */
    constexpr fixed_address(typename iterator::value_type address) noexcept;

    /**
     * @brief begin Iterator to the first value in the fixed_address.
     * @return
     */
    [[nodiscard]] constexpr auto begin() const noexcept -> iterator;

    /**
     * @brief end Iterator to the one-after last value in the fixed_address.
     * @return
     */
    [[nodiscard]] constexpr auto end() const noexcept -> iterator;

private:
    typename iterator::value_type m_address {};
};

template <std::integral T>
 constexpr fixed_address<T>::fixed_address(
    typename iterator::value_type address) noexcept
    : m_address {address} {}

template <std::integral T>
 constexpr auto fixed_address<T>::begin() const noexcept
    -> iterator {
    return iterator {m_address};
}

template <std::integral T>
 constexpr auto fixed_address<T>::end() const noexcept
    -> iterator {
    return iterator{static_cast<typename iterator::value_type>(m_address + 1)};
}

template <std::integral T>
 constexpr fixed_address<T>::iterator::iterator(
    value_type                current) noexcept
    : m_current {current} {}

template <std::integral T>
 constexpr fixed_address<T>::iterator::iterator(
    const iterator& other) noexcept
    : m_current {other.m_current} {}

template <std::integral T>
 constexpr auto
fixed_address<T>::iterator::operator=(const iterator& other) noexcept -> iterator& {
    m_current = other.m_current;
    return *this;
}

template <std::integral T>
 constexpr fixed_address<T>::iterator::iterator(
    iterator&& other) noexcept
    : m_current {other.m_current} {}

template <std::integral T>
 constexpr auto
fixed_address<T>::iterator::operator=(iterator&& other) noexcept -> iterator& {
    m_current = other.m_current;
    return *this;
}

template <std::integral T>
 constexpr fixed_address<T>::iterator::~iterator() noexcept =
    default;

template <std::integral T>
 constexpr auto
fixed_address<T>::iterator::operator*() const noexcept -> value_type {
    return m_current;
}

template <std::integral T>
 constexpr auto fixed_address<T>::iterator::operator++() noexcept
    -> iterator& {
    m_current += 1;
    return *this;
}

template <std::integral T>
 constexpr auto
fixed_address<T>::iterator::operator++(int) noexcept -> iterator {
    auto& original {*this};
    m_current += 1;
    return original;
}

template <std::integral T>
 constexpr auto
fixed_address<T>::iterator::operator!=(const iterator& other) const noexcept -> bool {
    return other.m_current != m_current;
}

template <std::integral T>
 constexpr auto
fixed_address<T>::iterator::operator==(const iterator& other) const noexcept -> bool {
    return other.m_current == m_current;
}
} // namespace muonpi

#endif // MUONPI_FIXEDADDRESS_H
