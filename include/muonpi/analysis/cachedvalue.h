#ifndef CACHEDVALUE_H
#define CACHEDVALUE_H

#include "muonpi/global.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <functional>
#include <numeric>

namespace muonpi {

/**
 * @brief holds a value and caches it appropriately
 * @param T the datatype of the value
 */
template <typename T>
class LIBMUONPI_PUBLIC cached_value {
public:
    /**
     * @brief cached_value
     * @param calculation The calculation to perform in order to get a new value
     * @param marker The criterium to determine whether a new value should be calculated or not. Return true for new calculation
     * @param invoke if true, calculation gets called immediatly upon construction
     */
    explicit cached_value(std::function<T()> calculation, std::function<bool()> marker, bool invoke = false)
        : m_calculation { calculation }
        , m_marker { marker }
        , m_value {}
    {
        if (invoke) {
            m_value = m_calculation();
        }
    }

    /**
     * @brief get Get the buffered value. If the function marker returns true, a new value is calculated.
     * @return The most recent value
     */
    [[nodiscard]] inline auto get() const -> T
    {
        if (m_marker()) {
            m_value = m_calculation();
        }
        return m_value;
    }

    /**
     * @brief operator() Get the buffered value. If the function marker returns true, a new value is calculated.
     * @return The most recent value
     */
    [[nodiscard]] inline auto operator()() const -> T
    {
        return get();
    }

private:
    std::function<T()> m_calculation {};
    std::function<bool()> m_marker;
    mutable T m_value {};
};

}
#endif // CACHEDVALUE_H
