#ifndef MUONPI_MULTIADDRESSRANGE_H
#define MUONPI_MULTIADDRESSRANGE_H

#include "muonpi/addressrange.h"
#include "muonpi/global.h"

#include <array>
#include <cstdint>

namespace muonpi {
/**
 * @brief The multi_address_range class.
 * Useful for defining address ranges of multiple device variants.
 * A common pattern for I2C devices is to have multiple variants of the same model with
 * identical functionality but different address ranges to allow for a larger number of used
 * devices.
 * @param N The number of address blocks in each address_range
 * @param M The number of address ranges
 * @param T The type of the addresses
 */
template <std::size_t N = 1, std::size_t M = 2, typename T = std::uint8_t>
requires std::is_integral_v<T> class LIBMUONPI_PUBLIC multi_address_range {
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
         * @brief iterator Construct an iterator on the address ranges.
         * this effectively initialises the begin iterator for the ranges.
         * @param ranges value_typehe ranges objects to use.
         */
        constexpr explicit iterator(std::array<address_range<N, value_type>, M> ranges) noexcept;

        /**
         * @brief iterator Construct an iterator on the address ranges.
         * This overload effectively creates an iterator to the end of the range.
         * @param ranges value_typehe ranges objects to use.
         */
        constexpr iterator(
            std::array<address_range<N, value_type>, M> ranges,
            bool) noexcept; // NOLINT(hicpp-named-parameter,readability-named-parameter)

        /**
         * @brief operator ++ Prefix increment operator
         */
        constexpr auto operator++() noexcept -> iterator&;

        /**
         * @brief operator ++ Postfix increment operator
         */
        constexpr auto operator++(int) noexcept -> iterator;

        /**
         * @brief operator ==
         * @param other
         * @return
         */
        [[nodiscard]] constexpr auto operator==(const iterator& other) const noexcept -> bool;

        /**
         * @brief operator !=
         * @param other
         * @return
         */
        [[nodiscard]] constexpr auto operator!=(const iterator& other) const noexcept -> bool;

        /**
         * @brief operator * Dereference the iterator
         * @return The current value
         */
        [[nodiscard]] constexpr auto operator*() const noexcept -> value_type;

    private:
        std::array<address_range<N, value_type>, M>     m_ranges {};
        std::size_t                                     n {};
        typename address_range<N, value_type>::iterator current;
        typename address_range<N, value_type>::iterator current_end;
    };

    /**
     * @brief multi_address_range Construct a multi address range
     * This is an object which represents multiple address iterators,
     * useful for I2C device variants with identical function but different address ranges.
     * @param ranges The different address_range objects.
     */
    constexpr explicit multi_address_range(
        std::array<address_range<N, typename iterator::value_type>, M> ranges) noexcept;

    /**
     * @brief begin Iterator to the first position in the range.
     * @return
     */
    [[nodiscard]] constexpr auto begin() const noexcept -> iterator;

    /**
     * @brief end Iterator to the next-after last item in the range.
     * @return
     */
    [[nodiscard]] constexpr auto end() const noexcept -> iterator;

private:
    std::array<address_range<N, typename iterator::value_type>, M> m_ranges {};
};

template <std::size_t N, std::size_t M, typename T>
requires std::is_integral_v<T> constexpr multi_address_range<N, M, T>::multi_address_range(
    std::array<address_range<N, typename iterator::value_type>, M> ranges) noexcept
    : m_ranges {ranges} {}

template <std::size_t N, std::size_t M, typename T>
requires std::is_integral_v<T> constexpr auto multi_address_range<N, M, T>::begin() const noexcept
    -> iterator {
    return iterator {m_ranges};
}

template <std::size_t N, std::size_t M, typename T>
requires std::is_integral_v<T> constexpr auto multi_address_range<N, M, T>::end() const noexcept
    -> iterator {
    return iterator {m_ranges, true};
}

template <std::size_t N, std::size_t M, typename T>
requires std::is_integral_v<T> constexpr multi_address_range<N, M, T>::iterator::iterator(
    std::array<address_range<N, value_type>, M> ranges) noexcept
    : m_ranges {ranges}
    , current {m_ranges[0].begin()}
    , current_end {m_ranges[0].end()} {}

template <std::size_t N, std::size_t M, typename T>
requires std::is_integral_v<T> constexpr multi_address_range<N, M, T>::iterator::iterator(
    std::array<address_range<N, value_type>, M> ranges,
    bool) noexcept // NOLINT(hicpp-named-parameter,readability-named-parameter)
    : m_ranges {ranges}
    , n {M}
    , current {m_ranges[M - 1].end()}
    , current_end {m_ranges[M - 1].end()} {}

template <std::size_t N, std::size_t M, typename T>
requires std::is_integral_v<T> constexpr auto
multi_address_range<N, M, T>::iterator::operator++() noexcept -> iterator& {
    ++current;
    if (current == current_end) {
        ++n;
        if (n < M) {
            current     = m_ranges[n].begin();
            current_end = m_ranges[n].end();
        }
    }
    return *this;
}

template <std::size_t N, std::size_t M, typename T>
requires std::is_integral_v<T> constexpr auto
multi_address_range<N, M, T>::iterator::operator++(int) noexcept -> iterator {
    auto& original {*this};
    ++current;
    if (current == current_end) {
        ++n;
        if (n < M) {
            current     = m_ranges[n].begin();
            current_end = m_ranges[n].end();
        }
    }
    return original;
}

template <std::size_t N, std::size_t M, typename T>
requires std::is_integral_v<T> constexpr auto
multi_address_range<N, M, T>::iterator::operator==(const iterator& other) const noexcept -> bool {
    return (n == other.n) && (current == other.current);
}

template <std::size_t N, std::size_t M, typename T>
requires std::is_integral_v<T> constexpr auto
multi_address_range<N, M, T>::iterator::operator!=(const iterator& other) const noexcept -> bool {
    return (n != other.n) || (current != other.current);
}

template <std::size_t N, std::size_t M, typename T>
requires std::is_integral_v<T> constexpr auto
multi_address_range<N, M, T>::iterator::operator*() const noexcept -> value_type {
    return *current;
}
} // namespace muonpi
#endif // MUONPI_MULTIADDRESSRANGE_H
