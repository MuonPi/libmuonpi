#ifndef DATASERIES_H
#define DATASERIES_H

#include "muonpi/global.h"

#include "muonpi/analysis/cachedvalue.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <functional>
#include <numeric>

namespace muonpi {

/**
 * @brief The data_series class
 * @param T The type of data points to process
 * @param N The maximum number of datapoints to store
 * @param Sample whether this should behave like a sample or a complete dataset (true for sample)
 */
template <typename T, std::size_t N, bool Sample = false>
class LIBMUONPI_PUBLIC data_series {
    static_assert(std::is_arithmetic<T>::value);

public:
    /**
     * @brief add Adds a value to the data series
     * @param value The value to add
     */
    void add(T value);

    /**
     * @brief mean Calculates the mean of all values. This value gets cached between data entries.
     * @return The mean
     */
    [[nodiscard]] auto mean() const -> T;

    /**
     * @brief mean Calculates the standard deviation of all values. This value gets cached between data entries.
     * @return The standard deviation
     */
    [[nodiscard]] auto stddev() const -> T;

    /**
     * @brief mean Calculates the variance of all values. This value gets cached between data entries.
     * Depending on the template parameter given with Sample, this calculates the variance of a sample
     * @return The variance
     */
    [[nodiscard]] auto variance() const -> T;

    /**
     * @brief entries Get the number of entries entered into this data series
     * @return Number of entries
     */
    [[nodiscard]] auto entries() const -> std::size_t;

    /**
     * @brief current Gets the most recent value
     * @return The most recent entry
     */
    [[nodiscard]] auto current() const -> T;

private:
    [[nodiscard]] inline auto private_mean() const -> T
    {
        const auto n { m_full ? N : (std::max<double>(m_index, 1.0)) };
        const auto end { m_full ? m_buffer.end() : m_buffer.begin() + m_index };
        const auto begin { m_buffer.begin() };

        return std::accumulate(begin, end, 0.0) / n;
    }

    [[nodiscard]] inline auto private_stddev() const -> T
    {
        return std::sqrt(variance());
    }

    [[nodiscard]] inline auto private_variance() const -> T
    {
        const auto n { m_full ? N : (std::max<double>(m_index, 1.0)) };
        const auto end { m_full ? m_buffer.end() : m_buffer.begin() + m_index };
        const auto begin { m_buffer.begin() };
        const auto denominator { Sample ? (n - 1.0) : n };
        const auto m { m_mean() };

        return 1.0 / (denominator)*std::inner_product(
                   begin, end, begin, 0.0, [](T const& x, T const& y) { return x + y; }, [m](T const& x, T const& y) { return (x - m) * (y - m); });
    }

    [[nodiscard]] inline auto dirty(bool& var) -> bool
    {
        if (var) {
            var = false;
            return true;
        }
        return false;
    }

    std::array<T, N> m_buffer { T {} };
    std::size_t m_index { 0 };
    bool m_full { false };
    bool m_mean_dirty { false };
    bool m_var_dirty { false };
    bool m_stddev_dirty { false };
    cached_value<T> m_mean { [this] { return private_mean(); }, [this] { return dirty(m_mean_dirty); } };
    cached_value<T> m_stddev { [this] { return private_stddev(); }, [this] { return dirty(m_stddev_dirty); } };
    cached_value<T> m_variance { [this] { return private_variance(); }, [this] { return dirty(m_var_dirty); } };
};

// +++++++++++++++++++++++++++++++
// implementation part starts here
// +++++++++++++++++++++++++++++++

template <typename T, std::size_t N, bool Sample>
void data_series<T, N, Sample>::add(T value)
{
    m_buffer[m_index] = value;
    m_mean_dirty = true;
    m_stddev_dirty = true;
    m_var_dirty = true;
    m_index = (m_index + 1) % N;
    if (m_index == 0) {
        m_full = true;
    }
}

template <typename T, std::size_t N, bool Sample>
auto data_series<T, N, Sample>::entries() const -> std::size_t
{
    return ((m_full) ? N : m_index);
}

template <typename T, std::size_t N, bool Sample>
auto data_series<T, N, Sample>::mean() const -> T
{
    return m_mean.get();
}

template <typename T, std::size_t N, bool Sample>
auto data_series<T, N, Sample>::stddev() const -> T
{
    return m_stddev.get();
}

template <typename T, std::size_t N, bool Sample>
auto data_series<T, N, Sample>::variance() const -> T
{
    return m_variance.get();
}

template <typename T, std::size_t N, bool Sample>
auto data_series<T, N, Sample>::current() const -> T
{
    return m_buffer[m_index];
}

}
#endif // DATASERIES_H
