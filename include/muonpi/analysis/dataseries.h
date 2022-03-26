#ifndef MUONPI_ANALYSIS_DATASERIES_H
#define MUONPI_ANALYSIS_DATASERIES_H

#include "muonpi/analysis/cachedvalue.h"
#include "muonpi/global.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <list>
#include <mutex>
#include <numeric>
#include <shared_mutex>

namespace muonpi {

/**
 * @brief The data_series class
 * @param T The type of data points to process
 * @param Sample whether this should behave like a sample or a complete dataset (true for sample)
 */
template <typename T>
class LIBMUONPI_PUBLIC data_series {
    static_assert(std::is_arithmetic<T>::value);

public:
    enum class mean_t
    {
        arithmetic,
        geometric,
        harmonic,
        quadratic
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
     * @brief rms calculathe the quadratic mean
     * @return the quadratic mean
     */
    [[deprecated]] [[nodiscard]] auto rms() const -> T;
    /**
     * @brief median Calculates the median of all values. This value gets cached between data
     * entries.
     * @return The median
     */
    [[nodiscard]] auto median() const -> T;

    /**
     * @brief stddev Calculates the standard deviation of all values. This value gets cached between
     * data entries. For small sample sizes of n <= 10, the factor will be 1/n, for sizes larger
     * than that, 1/(n-1). The standard deviation of sample sizes n = 1 is infinity.
     * @return The standard deviation
     */
    [[nodiscard]] auto stddev() const -> T;

    /**
     * @brief variance Calculates the variance of all values. This value gets cached between data
     * entries. For small sample sizes of n <= 10, the factor will be 1/n, for sizes larger than
     * that, 1/(n-1). The variance of sample sizes n = 1 is infinity.
     * @return The variance
     */
    [[nodiscard]] auto variance() const -> T;

    /**
     * @brief current Gets the most recent value
     * @return The most recent entry
     */
    [[nodiscard]] auto current() const -> T;

    /**
     * @brief min Gets the minimum value
     * @return The minimum
     */
    [[nodiscard]] auto min() const -> T;

    /**
     * @brief max Gets the maximum value
     * @return The maximum
     */
    [[nodiscard]] auto max() const -> T;

    /**
     * @brief sum Gets the sum of all values
     * @return The sum
     */
    [[nodiscard]] auto sum() const -> T;

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
    [[nodiscard]] auto private_mean(const mean_t& type) const -> T;

    [[nodiscard]] auto private_median() const -> T;

    [[nodiscard]] auto private_stddev() const -> T;

    [[nodiscard]] auto private_variance() const -> T;

    std::list<T> m_data {};
    std::size_t  m_n {0};

    cached_value<T> m_geometric_mean {[this] { return private_mean(mean_t::geometric); }};
    cached_value<T> m_arithmetic_mean {[this] { return private_mean(mean_t::arithmetic); }};
    cached_value<T> m_harmonic_mean {[this] { return private_mean(mean_t::harmonic); }};
    cached_value<T> m_quadratic_mean {[this] { return private_mean(mean_t::quadratic); }};
    cached_value<T> m_median {[this] { return private_median(); }};
    cached_value<T> m_stddev {[this] { return private_stddev(); }};
    cached_value<T> m_variance {[this] { return private_variance(); }};

    mutable std::shared_mutex m_mutex {};

    void mark_dirty() {
        m_arithmetic_mean.mark_dirty();
        m_geometric_mean.mark_dirty();
        m_harmonic_mean.mark_dirty();
        m_quadratic_mean.mark_dirty();
        m_median.mark_dirty();
        m_stddev.mark_dirty();
        m_variance.mark_dirty();
    }
};

// +++++++++++++++++++++++++++++++
// implementation part starts here
// +++++++++++++++++++++++++++++++

template <typename T>
data_series<T>::data_series(std::size_t n) noexcept
    : m_n {n} {}

template <typename T>
void data_series<T>::add(T value) {
    std::unique_lock<std::shared_mutex> lock {m_mutex};
    m_data.emplace_back(value);

    if (n() > m_n) {
        m_data.erase(m_data.begin());
    }

    mark_dirty();
}

template <typename T>
auto data_series<T>::data() const -> const std::list<T>& {
    std::shared_lock lock {m_mutex};
    return m_data;
}

template <typename T>
auto data_series<T>::n() const -> std::size_t {
    return m_data.size();
}

template <typename T>
auto data_series<T>::mean(const mean_t& type) const -> T {
    if (type == mean_t::geometric) {
        return m_geometric_mean.get();
    }
    if (type == mean_t::harmonic) {
        return m_harmonic_mean.get();
    }
    if (type == mean_t::quadratic) {
        return m_quadratic_mean.get();
    }
    return m_arithmetic_mean.get();
}

template <typename T>
auto data_series<T>::rms() const -> T {
    return mean(mean_t::quadratic);
}

template <typename T>
auto data_series<T>::median() const -> T {
    return m_median.get();
}

template <typename T>
auto data_series<T>::stddev() const -> T {
    return m_stddev.get();
}

template <typename T>
auto data_series<T>::variance() const -> T {
    return m_variance.get();
}

template <typename T>
auto data_series<T>::current() const -> T {
    return m_data.back();
}

template <typename T>
auto data_series<T>::min() const -> T {
    std::shared_lock lock {m_mutex};
    if (m_data.empty()) {
        return 0.0;
    }
    T min {std::numeric_limits<T>::max()};
    for (const auto& v : m_data) {
        if (v < min) {
            min = v;
        }
    }
    return min;
}

template <typename T>
auto data_series<T>::max() const -> T {
    std::shared_lock lock {m_mutex};
    if (m_data.empty()) {
        return 0.0;
    }
    T max {std::numeric_limits<T>::lowest()};
    for (const auto& v : m_data) {
        if (v > max) {
            max = v;
        }
    }
    return max;
}

template <typename T>
auto data_series<T>::sum() const -> T {
    std::shared_lock lock {m_mutex};
    return std::accumulate(std::begin(m_data), std::end(m_data), 0.0);
}

template <typename T>
void data_series<T>::reset() {
    std::unique_lock lock {m_mutex};
    m_data.clear();

    mark_dirty();
}

template <typename T>
void data_series<T>::reset(std::size_t n) {
    m_n = n;
    reset();
}

template <typename T>
auto data_series<T>::private_mean(const mean_t& type) const -> T {
    std::shared_lock lock {m_mutex};
    if (m_data.empty()) {
        return {};
    }
    if (type == mean_t::geometric) {
        return std::pow(std::accumulate(m_data.begin(), m_data.end(), 1.0, std::multiplies<T>()),
                        1.0 / static_cast<T>(n()));
    }
    if (type == mean_t::harmonic) {
        return static_cast<T>(n())
             / std::accumulate(m_data.begin(), m_data.end(), 0.0, [](const T& lhs, const T& rhs) {
                   return lhs + 1.0 / rhs;
               });
    }
    if (type == mean_t::quadratic) {
        return std::sqrt(std::inner_product(m_data.begin(), m_data.end(), m_data.begin(), 0.0)
                         / static_cast<T>(n()));
    }
    return std::accumulate(m_data.begin(), m_data.end(), 0.0) / static_cast<T>(n());
}

template <typename T>
auto data_series<T>::private_median() const -> T {
    std::shared_lock lock {m_mutex};
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

template <typename T>
auto data_series<T>::private_stddev() const -> T {
    std::shared_lock lock {m_mutex};
    if (m_data.empty()) {
        return {};
    }
    if (n() == 1) {
        return std::numeric_limits<double>::infinity();
    }
    return std::sqrt(variance());
}

template <typename T>
auto data_series<T>::private_variance() const -> T {
    std::shared_lock lock {m_mutex};
    if (m_data.empty()) {
        return {};
    }
    if (n() == 1) {
        return std::numeric_limits<double>::infinity();
    }
    const auto denominator {(n() > 10) ? (n() - 1.0) : n()};
    const auto m {mean()};

    return 1.0
         / (denominator)*std::inner_product(
               m_data.begin(),
               m_data.end(),
               m_data.begin(),
               0.0,
               [](T const& x, T const& y) { return x + y; },
               [m](T const& x, T const& y) { return (x - m) * (y - m); });
}

} // namespace muonpi

#endif // MUONPI_ANALYSIS_DATASERIES_H
