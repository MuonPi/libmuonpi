#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "muonpi/global.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <functional>
#include <numeric>

namespace muonpi {

/**
 * @brief The histogram class
 * @param T The type of each datapoints
 * @param C The type of the counter variable
 */
template <typename T = double, typename C = std::size_t>
class LIBMUONPI_PUBLIC histogram {
public:
    static_assert(std::is_integral<C>::value);
    static_assert(std::is_arithmetic<T>::value);

    struct bin {
        T lower {};
        T upper {};
        std::size_t count { 0 };
    };

    /**
     * @brief histogram
     * @param n The number of bins to use
     */
    explicit histogram(std::size_t n) noexcept;

    /**
     * @brief histogram Create a histogram with a fixed bin width. Note that the lower bound in this case will be assumed as 0.
     * @param width The width of each bin
     */
    explicit histogram(std::size_t n, T width) noexcept;

    /**
     * @brief histogram Create a histogram between two values.
     * @param lower The lower bound of the histogram
     * @param upper The upper bound
     */
    explicit histogram(std::size_t n, T lower, T upper) noexcept;

    /**
     * @brief fill fill the histogram with a series of data.
     * @param data the data to add
     */
    void fill(const std::vector<T>& data);

    /**
     * @brief add Adds a value to the histogram.
     * The value is deemed inside the histogram interval when it is >= lower and < upper.
     * If the value is exactly on the bound between two bins, the upper one is chosen.
     * @param value The value to add.
     */
    void add(T value);

    /**
     * @brief bins Get all bins
     * @return a const ref to the std::array cointaining the bins
     */
    [[nodiscard]] auto bins() const -> const std::vector<C>&;

    /**
     * @brief bins Get all bins
     * @return a const ref to the std::array cointaining the bins
     */
    [[nodiscard]] auto qualified_bins() const -> std::vector<bin>;

    /**
     * @brief width Get the binwidth of this histogram
     * @return
     */
    [[nodiscard]] auto width() const -> T;

    /**
     * @brief integral get the total number of entries
     * @return
     */
    [[nodiscard]] auto integral() const -> std::uint64_t;

    /**
     * @brief reset Completely empty the histogram without altering its defining values.
     */
    void reset();

    /**
     * @brief reset Completely empy the histogram and change the number of bins
     * @param n the new number of bins
     */
    void reset(std::size_t n);

    /**
     * @brief reset Completely empty the histogram and change its bin count and width
     * @param n the new number of bins
     * @param width the new bin width
     */
    void reset(std::size_t n, T width);

    /**
     * @brief reset Completely empty the histogram and change its bin count and upper/lower bound
     * @param n the new number of bins
     * @param lower the new lower bound of the histogram
     * @param upper the new upper bound of the histogram
     */
    void reset(std::size_t n, T lower, T upper);

    /**
     * @brief mode Calculate the mode (most common value) of the histogram
     * @return The mode
     */
    [[nodiscard]] auto mode() const -> T;

    /**
     * @brief percentile calculate the value of the percentile
     * @param percent The percentage value to check
     * @return the value associated with the percentage
     */
    [[nodiscard]] auto percentile(double percent) const -> T;

    /**
     * @brief mean Calculate the mean value of the histogram
     * @return the mean
     */
    [[nodiscard]] auto mean() const -> T;

    /**
     * @brief median Calculate the median value of the histogram
     * @return the median
     */
    [[nodiscard]] auto median() const -> T;

private:
    T m_lower {};
    T m_upper {};
    T m_width {};
    std::size_t m_n {};
    std::size_t m_lowest {};
    std::size_t m_highest {};

    std::vector<C> m_bins {};
};

// +++++++++++++++++++++++++++++++
// implementation part starts here
// +++++++++++++++++++++++++++++++

template <typename T, typename C>
histogram<T, C>::histogram(std::size_t n) noexcept
    : m_lower {}
    , m_upper {}
    , m_width {}
    , m_n { n }
{
    reset();
}

template <typename T, typename C>
histogram<T, C>::histogram(std::size_t n, T width) noexcept
    : m_lower {}
    , m_upper { width * n }
    , m_width { width }
    , m_n { n }
{
    reset();
}

template <typename T, typename C>
histogram<T, C>::histogram(std::size_t n, T lower, T upper) noexcept
    : m_lower { lower }
    , m_upper { upper }
    , m_width { (upper - lower) / static_cast<T>(n) }
    , m_n { n }
{
    reset();
}

template <typename T, typename C>
void histogram<T, C>::fill(const std::vector<T>& data)
{
    for (const auto& p : data) {
        add(p);
    }
}

template <typename T, typename C>
void histogram<T, C>::add(T value)
{
    if ((value < m_lower) || (value >= m_upper)) {
        return;
    }

    const std::size_t i { static_cast<std::size_t>(std::floor((value - m_lower) / m_width)) };

    m_bins[i]++;
}

template <typename T, typename C>
auto histogram<T, C>::bins() const -> const std::vector<C>&
{
    return m_bins;
}

template <typename T, typename C>
auto histogram<T, C>::qualified_bins() const -> std::vector<bin>
{
    std::vector<bin> bins;
    T last { m_lower };
    for (auto& b : m_bins) {
        bin current {};
        current.lower = last;
        last += m_width;
        current.upper = last;
        current.count = b;
        bins.emplace_back(std::move(current));
    }
    return bins;
}

template <typename T, typename C>
auto histogram<T, C>::width() const -> T
{
    return m_width;
}

template <typename T, typename C>
auto histogram<T, C>::integral() const -> std::uint64_t
{
    return std::accumulate(m_bins.begin(), m_bins.end(), 0);
}

template <typename T, typename C>
void histogram<T, C>::reset()
{
    m_bins.clear();
    m_bins.resize(m_n);
}

template <typename T, typename C>
void histogram<T, C>::reset(std::size_t n)
{
    m_n = n;
    reset();
}

template <typename T, typename C>
void histogram<T, C>::reset(std::size_t n, T width)
{
    m_n = n;
    m_lower = 0;
    m_upper = width * n;
    m_width = width;

    reset();
}

template <typename T, typename C>
void histogram<T, C>::reset(std::size_t n, T lower, T upper)
{
    m_n = n;
    m_lower = lower;
    m_upper = upper;
    m_width = (upper - lower) / static_cast<T>(n);

    reset();
}

template <typename T, typename C>
auto histogram<T, C>::mode() const -> T
{
    C maximum { std::numeric_limits<C>::min() };
    std::size_t max_i { 0 };

    for (std::size_t i { 0 }; i < m_bins.size(); i++) {
        if (maximum < m_bins.at(i)) {
            maximum = m_bins.at(i);
            max_i = i;
        }
    }

    return (static_cast<T>(max_i) + 0.5) * m_width;
}

template <typename T, typename C>
auto histogram<T, C>::mean() const -> T
{
    T total { 0 };
    T weighted { 0 };

    for (std::size_t i { 0 }; i < m_n; i++) {
        total += static_cast<T>(m_bins.at(i));
        weighted += static_cast<T>(m_bins.at(i)) * ((static_cast<T>(i) + 0.5) * m_width);
    }

    return weighted / total;
}

template <typename T, typename C>
auto histogram<T, C>::median() const -> T
{
    const std::size_t l { m_n / 2 };
    return m_lower + m_width * (static_cast<T>(l) + (static_cast<T>(m_n) * 0.5 - static_cast<T>(std::accumulate(m_bins.begin(), m_bins.begin() + l, 0))) / m_bins.at(l));
}

template <typename T, typename C>
auto histogram<T, C>::percentile(double percent) const -> T
{
    const auto total { static_cast<double>(integral()) };
    std::uint64_t lower {};
    auto edge { static_cast<std::uint64_t>(total * percent) };

    for (size_t i { 0 }; i < m_n; i++) {
        lower += m_bins.at(i);
        if (lower >= edge) {
            return m_lower + m_width * i;
        }
    }
    return m_upper;
}

}
#endif // HISTOGRAM_H
