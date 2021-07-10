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
 * @param N the number of items in the history
 * @param T the sampletime in milliseconds
 * @param Sample whether the statistics should be handled like a sample or a complete dataset
 */
template <std::size_t N, std::size_t T, bool Sample = false>
class LIBMUONPI_PUBLIC rate_measurement : public data_series<double, N, Sample> {
public:
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
    std::chrono::system_clock::time_point m_last { std::chrono::system_clock::now() };
};

// +++++++++++++++++++++++++++++++
// implementation part starts here
// +++++++++++++++++++++++++++++++

template <std::size_t N, std::size_t T, bool Sample>
void rate_measurement<N, T, Sample>::increase_counter()
{
    m_current_n = m_current_n + 1;
}

template <std::size_t N, std::size_t T, bool Sample>
auto rate_measurement<N, T, Sample>::step() -> bool
{
    return step(std::chrono::system_clock::now());
}

template <std::size_t N, std::size_t T, bool Sample>
auto rate_measurement<N, T, Sample>::step(const std::chrono::system_clock::time_point& now) -> bool
{
    if (static_cast<std::size_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last).count()) >= T) {
        m_last = now;
        constexpr double ms_to_s { 1000.0 };
        data_series<double, N, Sample>::add(static_cast<double>(m_current_n) * ms_to_s / static_cast<double>(T));
        m_current_n = 0;
        return true;
    }
    return false;
}

}
#endif // RATEMEASUREMENT_H
