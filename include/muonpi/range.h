#ifndef MUONPI_RANGE_H
#define MUONPI_RANGE_H

#include <concepts>
#include <cstdint>
#include <iterator>

namespace muonpi {

template <typename T = ssize_t>
requires std::is_integral_v<T>
    /**
     * @brief The range class
     */
    class range {
public:
    /**
     * @brief The iterator class
     */
    class iterator {
    public:
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = value_type*;
        using reference         = value_type&;
        using iterator_category = std::bidirectional_iterator_tag;

        /**
         * @brief iterator Create an iterator with step size step and current value current
         * @param step The step size
         * @param current The current value
         */
        constexpr iterator(difference_type step, value_type current) noexcept;

        /**
         * @brief iterator Copy constructor
         * @param other
         */
        constexpr iterator(const iterator& other);

        /**
         * @brief operator = copy assignment
         * @param other
         */
        constexpr auto operator=(const iterator& other) -> iterator&;

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
        constexpr ~iterator();

        /**
         * @brief operator * Dereference the iterator
         * @return The current value
         */
        [[nodiscard]] constexpr auto operator*() const -> value_type;
        /**
         * @brief operator ++ Prefix increment the iterator
         */
        constexpr auto operator++() -> iterator&;
        /**
         * @brief operator ++ Postfix increment the iterator
         * @return
         */
        constexpr auto operator++(int) -> iterator;
        /**
         * @brief operator -- Prefix decrement the iterator
         */
        constexpr auto operator--() -> iterator&;
        /**
         * @brief operator -- Postfix decrement the iterator
         * @return
         */
        constexpr auto operator--(int) -> iterator;

        /**
         * @brief operator !=
         * @param other
         * @return
         */
        [[nodiscard]] constexpr auto operator!=(const iterator& other) const -> bool;
        /**
         * @brief operator ==
         * @param other
         * @return
         */
        [[nodiscard]] constexpr auto operator==(const iterator& other) const -> bool;

    private:
        difference_type m_step {};
        value_type      m_current {};
    };

    /**
     * @brief range Create a range object
     * @param begin The first value in the range
     * @param end The one after last value of the range. (exclusive last value)
     * @param step The step size
     * @throws std::runtime_error in case the choice of step size would cause an infinite loop
     */
    constexpr range(typename iterator::value_type      begin,
                    typename iterator::value_type      end,
                    typename iterator::difference_type step);

    /**
     * @brief range Creae a range object with automatic step size.
     * The step size is always 1 with the correct direction.
     * @param begin The first value in the range.
     * @param end The one after last value of the range. (exclusive last value)
     */
    constexpr range(typename iterator::value_type begin, typename iterator::value_type end);

    /**
     * @brief range Create a range object with automatic step size and start.
     * The start value is always chosen to be 0
     * The step size is always 1, with the correct direction.
     * @param end  The one after last value of the range. (exclusive last value)
     */
    constexpr explicit range(typename iterator::value_type end);

    /**
     * @brief begin Iterator to the first value in the range.
     * @return
     */
    [[nodiscard]] constexpr auto begin() const -> iterator;

    /**
     * @brief end Iterator to the one-after last value in the range.
     * @return
     */
    [[nodiscard]] constexpr auto end() const -> iterator;

private:
    typename iterator::value_type      m_begin {};
    typename iterator::value_type      m_end {};
    typename iterator::difference_type m_step {};
};

template <typename T>
requires std::is_integral_v<T> constexpr range<T>::range(typename iterator::value_type      begin,
                                                         typename iterator::value_type      end,
                                                         typename iterator::difference_type step)
    : m_begin {begin}
    , m_end {end}
    , m_step {step} {
    if ((step == 0) || ((end - begin) % step != 0)
        || (static_cast<typename iterator::difference_type>(end - begin) * step < 0)) {
        throw std::runtime_error {"Unsafe step size. Will miss the last value."};
    }
}

template <typename T>
requires std::is_integral_v<T> constexpr range<T>::range(typename iterator::value_type begin,
                                                         typename iterator::value_type end)
    : range {begin, end, (end > begin) ? 1 : -1} {}

template <typename T>
requires std::is_integral_v<T> constexpr range<T>::range(typename iterator::value_type end)
    : range {0, end, (end > 0) ? 1 : -1} {}

template <typename T>
requires std::is_integral_v<T> constexpr auto range<T>::begin() const -> iterator {
    return iterator {m_step, m_begin};
}

template <typename T>
requires std::is_integral_v<T> constexpr auto range<T>::end() const -> iterator {
    return iterator {m_step, m_end};
}

template <typename T>
requires std::is_integral_v<T> constexpr range<T>::iterator::iterator(difference_type step,
                                                                      value_type current) noexcept
    : m_step {step}
    , m_current {current} {}

template <typename T>
requires std::is_integral_v<T> constexpr range<T>::iterator::iterator(const iterator& other)
    : m_step {other.m_step}
    , m_current {other.m_current} {}

template <typename T>
requires std::is_integral_v<T> constexpr auto range<T>::iterator::operator=(const iterator& other)
    -> iterator& {
    m_step    = other.m_step;
    m_current = other.m_current;
    return *this;
}

template <typename T>
requires std::is_integral_v<T> constexpr range<T>::iterator::iterator(iterator&& other) noexcept
    : m_step {other.m_step}
    , m_current {other.m_current} {}

template <typename T>
requires std::is_integral_v<T> constexpr auto
range<T>::iterator::operator=(iterator&& other) noexcept -> iterator& {
    m_step    = other.m_step;
    m_current = other.m_current;
    return *this;
}

template <typename T>
requires std::is_integral_v<T> constexpr range<T>::iterator::~iterator() = default;

template <typename T>
requires std::is_integral_v<T> constexpr auto range<T>::iterator::operator*() const -> value_type {
    return m_current;
}

template <typename T>
requires std::is_integral_v<T> constexpr auto range<T>::iterator::operator++() -> iterator& {
    m_current += m_step;
    return *this;
}

template <typename T>
requires std::is_integral_v<T> constexpr auto range<T>::iterator::operator++(int) -> iterator {
    auto& original {*this};
    m_current += m_step;
    return original;
}

template <typename T>
requires std::is_integral_v<T> constexpr auto range<T>::iterator::operator--() -> iterator& {
    m_current -= m_step;
    return *this;
}

template <typename T>
requires std::is_integral_v<T> constexpr auto range<T>::iterator::operator--(int) -> iterator {
    auto& original {*this};
    m_current -= m_step;
    return original;
}

template <typename T>
requires std::is_integral_v<T> constexpr auto
range<T>::iterator::operator!=(const iterator& other) const -> bool {
    return other.m_current != m_current;
}

template <typename T>
requires std::is_integral_v<T> constexpr auto
range<T>::iterator::operator==(const iterator& other) const -> bool {
    return other.m_current == m_current;
}
} // namespace muonpi
#endif // MUONPI_RANGE_H
