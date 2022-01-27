#ifndef RATEMEASUREMENT_H
#define RATEMEASUREMENT_H

#include "muonpi/global.h"

#include "muonpi/analysis/dataseries.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <functional>
#include <numeric>

namespace muonpi {

/**
 * @brief The rate_measurement class
 * @param T the sampletime in milliseconds
 * @param Sample whether the statistics should be handled like a sample or a complete dataset
 */
template <typename T, bool Sample = false>
class LIBMUONPI_PUBLIC rate_measurement : public data_series<T, Sample> {
public:
    explicit rate_measurement(std::size_t n, std::chrono::seconds t) noexcept;
    /**
     * @brief increase_counter Increases the counter in the current interval
     */
    void increase_counter();

    /**
     * @brief step Called periodically. Internally calls step(now);
     * @return True if the timeout was reached and the rates have been determined in this step
     */
    auto step() -> bool;

    /**
     * @brief step Called periodically
     * @param now the time point when the method was called
     * @return True if the timeout was reached and the rates have been determined in this step
     */
    auto step(const std::chrono::system_clock::time_point& now) -> bool;

private:
    std::size_t m_current_n { 0 };
    std::chrono::seconds m_t {};
    std::chrono::system_clock::time_point m_last { std::chrono::system_clock::now() };
};

// +++++++++++++++++++++++++++++++
// implementation part starts here
// +++++++++++++++++++++++++++++++

template <typename T, bool Sample>
rate_measurement<T, Sample>::rate_measurement(std::size_t n, std::chrono::seconds t) noexcept
    : data_series<T, Sample>(n)
    , m_t { t }
{
}

template <typename T, bool Sample>
void rate_measurement<T, Sample>::increase_counter()
{
    m_current_n++;
}

template <typename T, bool Sample>
auto rate_measurement<T, Sample>::step() -> bool
{
    return step(std::chrono::system_clock::now());
}

template <typename T, bool Sample>
auto rate_measurement<T, Sample>::step(const std::chrono::system_clock::time_point& now) -> bool
{
    if ((now - m_last) >= m_t) {
        m_last = now;
        data_series<T, Sample>::add(static_cast<T>(m_current_n) / static_cast<T>(m_t.count()));
        m_current_n = 0;
        return true;
    }
    return false;
}

} // namespace muonpi
#endif // RATEMEASUREMENT_H
