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
 * @param N the number of bins to use
 * @param T The type of each datapoints
 * @param C The type of the counter variable
 */
template <std::size_t N, typename T = double, typename C = std::size_t>
class LIBMUONPI_PUBLIC histogram {
public:
    static_assert(std::is_integral<C>::value);
    static_assert(std::is_arithmetic<T>::value);

    struct bin {
        T lower {};
        T upper {};
        std::size_t count { 0 };
    };

    explicit histogram();

    /**
     * @brief histogram Create a histogram with a fixed bin width. Note that the lower bound in this case will be assumed as 0.
     * @param width The width of each bin
     */
    explicit histogram(T width);

    /**
     * @brief histogram Create a histogram between two values.
     * @param lower The lower bound of the histogram
     * @param upper The upper bound
     */
    explicit histogram(T lower, T upper);

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
    [[nodiscard]] auto bins() const -> const std::array<C, N>&;

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
     * @brief clear reset all bins to 0
     */
    void clear();

private:
    T m_lower {};
    T m_upper {};
    T m_width {};
    std::array<C, N> m_bins {};
};

// +++++++++++++++++++++++++++++++
// implementation part starts here
// +++++++++++++++++++++++++++++++

template <std::size_t N, typename T, typename C>
histogram<N, T, C>::histogram()
    : m_lower {}
    , m_upper {}
    , m_width {}
{
}

template <std::size_t N, typename T, typename C>
histogram<N, T, C>::histogram(T width)
    : m_lower {}
    , m_upper { width * N }
    , m_width { width }
{
}

template <std::size_t N, typename T, typename C>
histogram<N, T, C>::histogram(T lower, T upper)
    : m_lower { lower }
    , m_upper { upper }
    , m_width { (upper - lower) / static_cast<T>(N) }
{
}

template <std::size_t N, typename T, typename C>
void histogram<N, T, C>::add(T value)
{
    if ((value < m_lower) || (value >= m_upper)) {
        return;
    }

    const std::size_t i { static_cast<std::size_t>(std::floor((value - m_lower) / m_width)) };

    m_bins[i]++;
}

template <std::size_t N, typename T, typename C>
auto histogram<N, T, C>::bins() const -> const std::array<C, N>&
{
    return m_bins;
}

template <std::size_t N, typename T, typename C>
auto histogram<N, T, C>::qualified_bins() const -> std::vector<bin>
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

template <std::size_t N, typename T, typename C>
auto histogram<N, T, C>::width() const -> T
{
    return m_width;
}

template <std::size_t N, typename T, typename C>
auto histogram<N, T, C>::integral() const -> std::uint64_t
{
    std::uint64_t total {};
    for (const auto& n : m_bins) {
        total += n;
    }
    return total;
}

template <std::size_t N, typename T, typename C>
void histogram<N, T, C>::clear()
{
    for (auto& n : m_bins) {
        n = 0;
    }
}

}
#endif // HISTOGRAM_H
