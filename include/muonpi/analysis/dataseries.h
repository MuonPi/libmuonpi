#ifndef DATASERIES_H
#define DATASERIES_H

#include "muonpi/global.h"

#include "muonpi/analysis/cachedvalue.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <list>
#include <numeric>
#include <shared_mutex>

namespace muonpi {

/**
 * @brief The data_series class
 * @param T The type of data points to process
 * @param Sample whether this should behave like a sample or a complete dataset (true for sample)
 */
template <typename T, bool Sample = false>
class LIBMUONPI_PUBLIC data_series {
    static_assert(std::is_arithmetic<T>::value);

public:
    enum class mean_t {
        arithmetic,
        geometric,
        harmonic
    };

    /**
     * @brief data_series
     * @param n the maximum number of entries to hold
     */
    explicit data_series(std::size_t n) noexcept;

    /**
     * @brief add Adds a value to the data series
     * @param value The value to add
     */
    void add(T value);

    /**
     * @brief mean Calculates the mean of all values. This value gets cached between data entries.
     * @return The mean
     */
    [[nodiscard]] auto mean(const mean_t& type = mean_t::arithmetic) const -> T;

    /**
     * @brief median Calculates the median of all values. This value gets cached between data entries.
     * @return The median
     */
    [[nodiscard]] auto median() const -> T;

    /**
     * @brief stddev Calculates the standard deviation of all values. This value gets cached between data entries.
     * @return The standard deviation
     */
    [[nodiscard]] auto stddev() const -> T;

    /**
     * @brief variance Calculates the variance of all values. This value gets cached between data entries.
     * Depending on the template parameter given with Sample, this calculates the variance of a sample
     * @return The variance
     */
    [[nodiscard]] auto variance() const -> T;

    /**
     * @brief rms Calculates the rms (Root Mean Square) of all values. This value gets cached between data entries.
     * @return The RMS
     */
    [[nodiscard]] auto rms() const -> T;

    /**
     * @brief current Gets the most recent value
     * @return The most recent entry
     */
    [[nodiscard]] auto current() const -> T;

    /**
     * @brief n Get the number of entries entered into this data series
     * @return Number of entries
     */
    [[nodiscard]] auto n() const -> std::size_t;

    /**
     * @brief data Get the data
     * @return const ref to the data vector
     */
    [[nodiscard]] auto data() const -> const std::list<T>&;

    /**
     * @brief reset Clear the whole dataset and leave the size unchanged
     */
    void reset();

    /**
     * @brief reset Clear the whole dataset and resize the sample size
     * @param n The new sample size
     */
    void reset(std::size_t n);

private:
    [[nodiscard]] inline auto private_mean(const mean_t& type) const -> T
    {
        std::shared_lock lock { m_mutex };
        if (m_data.empty()) {
            return {};
        }
        if (type == mean_t::geometric) {
            return std::pow(std::accumulate(m_data.begin(), m_data.end(), 0.0, std::multiplies<T>()), 1.0 / static_cast<T>(n()));
        } else if (type == mean_t::harmonic) {
            return static_cast<T>(n()) / std::accumulate(m_data.begin(), m_data.end(), 0.0, [](const T& lhs, const T& rhs) { return lhs + 1.0 / rhs; });
        }
        return std::accumulate(m_data.begin(), m_data.end(), 0.0) / static_cast<T>(n());
    }

    [[nodiscard]] inline auto private_median() const -> T
    {
        std::shared_lock lock { m_mutex };
        if (m_data.empty()) {
            return {};
        }
        std::vector<T> sorted {};
        sorted.resize(n());

        std::copy(m_data.begin(), m_data.end(), sorted.begin());

        std::sort(sorted.begin(), sorted.end());

        if (n() % 2 == 0) {
            return (sorted.at(n() / 2) + sorted.at(n() / 2 + 1)) / 2.0;
        }
        return sorted.at(n() / 2);
    }

    [[nodiscard]] inline auto private_stddev() const -> T
    {
        std::shared_lock lock { m_mutex };
        if (m_data.empty()) {
            return {};
        }
        return std::sqrt(variance());
    }

    [[nodiscard]] inline auto private_variance() const -> T
    {
        std::shared_lock lock { m_mutex };
        if (m_data.empty()) {
            return {};
        }
        const auto denominator { Sample ? (n() - 1.0) : n() };
        const auto m { mean() };

        return 1.0 / (denominator)*std::inner_product(
                   m_data.begin(), m_data.end(), m_data.begin(), 0.0, [](T const& x, T const& y) { return x + y; }, [m](T const& x, T const& y) { return (x - m) * (y - m); });
    }

    [[nodiscard]] inline auto private_rms() const -> T
    {
        std::shared_lock lock { m_mutex };
        if (m_data.empty()) {
            return {};
        }
        return std::sqrt(std::inner_product(m_data.begin(), m_data.end(), m_data.begin(), 0) / static_cast<T>(n()));
    }

    std::list<T> m_data {};
    std::size_t m_n { 0 };

    cached_value<T> m_geometric_mean { [this] { return private_mean(mean_t::geometric); } };
    cached_value<T> m_arithmetic_mean { [this] { return private_mean(mean_t::arithmetic); } };
    cached_value<T> m_harmonic_mean { [this] { return private_mean(mean_t::harmonic); } };
    cached_value<T> m_median { [this] { return private_median(); } };
    cached_value<T> m_stddev { [this] { return private_stddev(); } };
    cached_value<T> m_variance { [this] { return private_variance(); } };
    cached_value<T> m_rms { [this] { return private_rms(); } };

    mutable std::shared_mutex m_mutex {};

    void mark_dirty()
    {
        m_arithmetic_mean.mark_dirty();
        m_geometric_mean.mark_dirty();
        m_harmonic_mean.mark_dirty();
        m_median.mark_dirty();
        m_stddev.mark_dirty();
        m_variance.mark_dirty();
        m_rms.mark_dirty();
    }
};

// +++++++++++++++++++++++++++++++
// implementation part starts here
// +++++++++++++++++++++++++++++++

template <typename T, bool Sample>
data_series<T, Sample>::data_series(std::size_t n) noexcept
    : m_n { n }
{
}

template <typename T, bool Sample>
void data_series<T, Sample>::add(T value)
{
    std::unique_lock lock { m_mutex };
    m_data.emplace_back(value);

    if (n() > m_n) {
        m_data.erase(m_data.begin());
    }

    mark_dirty();
}

template <typename T, bool Sample>
auto data_series<T, Sample>::data() const -> const std::list<T>&
{
    std::shared_lock lock { m_mutex };
    return m_data;
}

template <typename T, bool Sample>
auto data_series<T, Sample>::n() const -> std::size_t
{
    return m_data.size();
}

template <typename T, bool Sample>
auto data_series<T, Sample>::mean(const mean_t& type) const -> T
{
    if (type == mean_t::geometric) {
        return m_geometric_mean.get();
    } else if (type == mean_t::harmonic) {
        return m_harmonic_mean.get();
    }
    return m_arithmetic_mean.get();
}

template <typename T, bool Sample>
auto data_series<T, Sample>::median() const -> T
{
    return m_median.get();
}

template <typename T, bool Sample>
auto data_series<T, Sample>::stddev() const -> T
{
    return m_stddev.get();
}

template <typename T, bool Sample>
auto data_series<T, Sample>::variance() const -> T
{
    return m_variance.get();
}

template <typename T, bool Sample>
auto data_series<T, Sample>::rms() const -> T
{
    return m_rms.get();
}

template <typename T, bool Sample>
auto data_series<T, Sample>::current() const -> T
{
    return m_data.back();
}

template <typename T, bool Sample>
void data_series<T, Sample>::reset()
{
    std::unique_lock lock { m_mutex };
    m_data.clear();

    mark_dirty();
}

template <typename T, bool Sample>
void data_series<T, Sample>::reset(std::size_t n)
{
    m_n = n;
    reset();
}

}

#endif // DATASERIES_H
