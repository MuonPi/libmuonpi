#ifndef MUONPI_ANALYSIS_CACHEDVALUE_H
#define MUONPI_ANALYSIS_CACHEDVALUE_H

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
template <typename T, typename... P>
class LIBMUONPI_PUBLIC cached_value {
public:
    /**
     * @brief cached_value
     * @param calculation The calculation to perform in order to get a new value
     */
    explicit cached_value(std::function<T(P...)> calculation)
        : m_calculation {calculation} {}

    /**
     * @brief get Get the buffered value. If the function marker returns true, a new value is
     * calculated.
     * @return The most recent value
     */
    [[nodiscard]] inline auto get(P... params) const -> T {
        if (m_dirty) {
            m_value = m_calculation(params...);
            m_dirty = false;
        }
        return m_value;
    }

    /**
     * @brief operator() Get the buffered value. If the function marker returns true, a new value is
     * calculated.
     * @return The most recent value
     */
    [[nodiscard]] inline auto operator()(P... params) const -> T {
        return get(params...);
    }

    inline void mark_dirty() {
        m_dirty = true;
    }

private:
    std::function<T(P...)> m_calculation {};
    mutable bool           m_dirty {true};
    mutable T              m_value {};
};

} // namespace muonpi
#endif // MUONPI_ANALYSIS_CACHEDVALUE_H
