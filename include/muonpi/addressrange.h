#ifndef MUONPI_ADDRESSRANGE_H
#define MUONPI_ADDRESSRANGE_H

#include "muonpi/global.h"

#include <array>
#include <concepts>
#include <cstdint>
#include <iterator>

namespace muonpi {
template <std::size_t N = 1, typename T = std::uint8_t>
requires std::is_integral_v<T>
    /**
     * @brief The address_range class
     */
    class LIBMUONPI_PUBLIC address_range {
public:
    /**
     * @brief The mask_range struct
     * Represent a single consecutive address mask block
     */
    struct mask_range {
        using value_type = T;
        constexpr static value_type n_bits {sizeof(value_type) * 8};
        value_type                  mask {};
        value_type                  start_bit {};
        value_type                  end_bit {n_bits};

        [[nodiscard]] constexpr auto operator<(const mask_range& other) const -> bool {
            return mask < other.mask;
        }

        [[nodiscard]] constexpr auto operator>(const mask_range& other) const -> bool {
            return mask > other.mask;
        }

        /**
         * @brief mask_range
         * @param m The address mask to use
         */
        constexpr mask_range(value_type m) noexcept
            : mask {m} {
            bool started {false};
            for (value_type i {0}; i < n_bits; ++i) {
                if (!started) {
                    if ((m & 1 << i) != 0) {
                        start_bit = i;
                        started   = true;
                    }
                } else {
                    if ((m & 1 << i) == 0) {
                        end_bit = i;
                        break;
                    }
                }
            }
        }

        /**
         * @brief construct Construct a valid address block from an aligned bit value.
         * This function will select the appropriate number of bits from the input
         * and shifts them to the appropriate position. Input should always be right aligned.
         * Example:
         * input: 0b00000101
         * mask: 0b01110000
         * return: 0b01010000
         * @param input The bit value.
         * @return
         */
        [[nodiscard]] constexpr auto construct(value_type input) const noexcept -> value_type {
            return (input << start_bit) & mask;
        }

        /**
         * @brief align Alignes an input bit pattern for the next mask
         * by shifting the bits right by the appropriate amount
         * Example:
         * input: 0b00110111
         * mask: 0b00011100
         * output: 0b00000110
         *
         * @param input
         * @return
         */
        [[nodiscard]] constexpr auto align(value_type input) const noexcept -> value_type {
            return input >> (end_bit - start_bit);
        }

        /**
         * @brief bits The number of bits set in the mask.
         * @return
         */
        [[nodiscard]] constexpr auto bits() const noexcept -> value_type {
            return end_bit - start_bit;
        }
    };
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
         * @param base The base address. with all mask bits set to 0.
         * @param current The current value
         * @param masks A list of bit masks of consecutive address blocks.
         * The order is important:
         * The lowest value bit mask should come before the higher value bit masks.
         * Example:
         * {0b00000011, 0b00011000}
         */
        explicit constexpr iterator(value_type                base,
                                    value_type                current,
                                    std::array<mask_range, N> masks) noexcept;

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
        value_type                m_base {};
        value_type                m_current {};
        std::array<mask_range, N> m_masks {};
    };

    /**
     * @brief address_range Creae a address_range object
     * @param base_address The base address so that
     * base_address & ~masks == base_address
     * Which is the lowst possible address.
     * The highest possible address is
     * base_address | masks
     * @param masks A list of bit masks of consecutive address blocks.
     * The order is important:
     * The lowest value bit mask should come before the higher value bit masks.
     * Example:
     * {0b00000011, 0b00011000}
     */
    constexpr address_range(typename iterator::value_type base_address,
                            std::array<mask_range, N>     masks) noexcept;

    /**
     * @brief begin Iterator to the first value in the address_range.
     * @return
     */
    [[nodiscard]] constexpr auto begin() const noexcept -> iterator;

    /**
     * @brief end Iterator to the one-after last value in the address_range.
     * @return
     */
    [[nodiscard]] constexpr auto end() const noexcept -> iterator;

private:
    typename iterator::value_type m_base_address {};
    std::array<mask_range, N>     m_masks {};
};

template <std::size_t N, typename T>
requires std::is_integral_v<T> constexpr address_range<N, T>::address_range(
    typename iterator::value_type base_address,
    std::array<mask_range, N>     masks) noexcept
    : m_base_address {base_address}
    , m_masks {std::move(masks)} {
    std::sort(std::begin(m_masks), std::end(m_masks));
}

template <std::size_t N, typename T>
requires std::is_integral_v<T> constexpr auto address_range<N, T>::begin() const noexcept
    -> iterator {
    return iterator {m_base_address, 0, m_masks};
}

template <std::size_t N, typename T>
requires std::is_integral_v<T> constexpr auto address_range<N, T>::end() const noexcept
    -> iterator {
    typename iterator::value_type output {1};
    for (const auto& mask : m_masks) {
        output = output << mask.bits();
    }
    return iterator {m_base_address, output, m_masks};
}

template <std::size_t N, typename T>
requires std::is_integral_v<T> constexpr address_range<N, T>::iterator::iterator(
    value_type                base,
    value_type                current,
    std::array<mask_range, N> masks) noexcept
    : m_base {base}
    , m_current {current}
    , m_masks {masks} {}

template <std::size_t N, typename T>
requires std::is_integral_v<T> constexpr address_range<N, T>::iterator::iterator(
    const iterator& other) noexcept
    : m_base {other.m_base}
    , m_current {other.m_current}
    , m_masks {other.m_masks} {}

template <std::size_t N, typename T>
requires std::is_integral_v<T> constexpr auto
address_range<N, T>::iterator::operator=(const iterator& other) noexcept -> iterator& {
    m_base    = other.m_base;
    m_current = other.m_current;
    m_masks   = other.m_masks;
    return *this;
}

template <std::size_t N, typename T>
requires std::is_integral_v<T> constexpr address_range<N, T>::iterator::iterator(
    iterator&& other) noexcept
    : m_base {other.m_base}
    , m_current {other.m_current}
    , m_masks {other.m_masks} {}

template <std::size_t N, typename T>
requires std::is_integral_v<T> constexpr auto
address_range<N, T>::iterator::operator=(iterator&& other) noexcept -> iterator& {
    m_base    = other.m_base;
    m_current = other.m_current;
    m_masks   = other.m_masks;
    return *this;
}

template <std::size_t N, typename T>
requires std::is_integral_v<T> constexpr address_range<N, T>::iterator::~iterator() noexcept =
    default;

template <std::size_t N, typename T>
requires std::is_integral_v<T> constexpr auto
address_range<N, T>::iterator::operator*() const noexcept -> value_type {
    auto       input {m_current};
    value_type output {};
    for (const auto& mask : m_masks) {
        output |= mask.construct(input);
        input = mask.align(input);
    }
    return m_base | output;
}

template <std::size_t N, typename T>
requires std::is_integral_v<T> constexpr auto address_range<N, T>::iterator::operator++() noexcept
    -> iterator& {
    m_current += 1;
    return *this;
}

template <std::size_t N, typename T>
requires std::is_integral_v<T> constexpr auto
address_range<N, T>::iterator::operator++(int) noexcept -> iterator {
    auto& original {*this};
    m_current += 1;
    return original;
}

template <std::size_t N, typename T>
requires std::is_integral_v<T> constexpr auto
address_range<N, T>::iterator::operator!=(const iterator& other) const noexcept -> bool {
    return other.m_current != m_current;
}

template <std::size_t N, typename T>
requires std::is_integral_v<T> constexpr auto
address_range<N, T>::iterator::operator==(const iterator& other) const noexcept -> bool {
    return other.m_current == m_current;
}
} // namespace muonpi
#endif // MUONPI_ADDRESSRANGE_H
