#ifndef MUONPI_ANALYSIS_RATEMEASUREMENT_H
#define MUONPI_ANALYSIS_RATEMEASUREMENT_H

#include "muonpi/analysis/dataseries.h"
#include "muonpi/global.h"

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
 */
template <typename T>
class LIBMUONPI_PUBLIC rate_measurement : public data_series<T> {
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
    auto step(std::chrono::system_clock::time_point now) -> bool;

private:
    std::size_t                           m_current_n {0};
    std::chrono::seconds                  m_t {};
    std::chrono::system_clock::time_point m_last {std::chrono::system_clock::now()};
};

// +++++++++++++++++++++++++++++++
// implementation part starts here
// +++++++++++++++++++++++++++++++

template <typename T>
rate_measurement<T>::rate_measurement(std::size_t n, std::chrono::seconds t) noexcept
    : data_series<T>(n)
    , m_t {t} {}

template <typename T>
void rate_measurement<T>::increase_counter() {
    m_current_n++;
}

template <typename T>
auto rate_measurement<T>::step() -> bool {
    return step(std::chrono::system_clock::now());
}

template <typename T>
auto rate_measurement<T>::step(std::chrono::system_clock::time_point now) -> bool {
    const auto diff {now - m_last};
    if (diff >= m_t) {
        m_last = now;
        data_series<T>::add(
            static_cast<T>(m_current_n) * 1.0e6
            / static_cast<T>(std::chrono::duration_cast<std::chrono::microseconds>(diff).count()));
        m_current_n = 0;
        return true;
    }
    return false;
}

} // namespace muonpi
#endif // MUONPI_ANALYSIS_RATEMEASUREMENT_H
