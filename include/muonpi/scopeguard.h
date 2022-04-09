#ifndef UTILITY_SCOPEGUARD_H
#define UTILITY_SCOPEGUARD_H

#include "muonpi/global.h"

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>

namespace muonpi {
/**
 * @brief The scope_guard class
 * executes a cleanup function upon destruction unless dismiss() is called.
 */
class LIBMUONPI_PUBLIC scope_guard {
public:
    /**
     * @brief scope_guard
     * @param cleanup The function to be called on non dismissed destruction
     */
    explicit scope_guard(std::function<void()> cleanup);

    /**
     * @brief scope_guard
     * @param cleanup The function to be called on non dismissed destruction
     * @param dismiss The function to be called on dismissed destruction
     */
    scope_guard(std::function<void()> cleanup, std::function<void()> dismiss);

    scope_guard(scope_guard&& other) noexcept;

    scope_guard()                   = delete;
    scope_guard(const scope_guard&) = delete;

    auto operator=(scope_guard&&) -> scope_guard& = delete;
    auto operator=(const scope_guard&) -> scope_guard& = delete;

    ~scope_guard();

    /**
     * @brief dismiss Dismiss the guard, so the cleanup function will not be executed.
     */
    void dismiss();

private:
    /**
     * @brief dissolve moves the function from the scope_guard.
     * Intended to only be called from another scope_guard object.
     * @return the function object.
     */
    [[nodiscard]] auto dissolve() -> std::function<void()>;

    std::function<void()> m_cleanup;
    std::function<void()> m_dismiss {[] {}};
};

[[nodiscard]] auto wait_for(std::condition_variable&  cv,
                            std::chrono::milliseconds interval,
                            std::chrono::seconds      total_wait) -> bool;

} // namespace muonpi

#endif // UTILITY_SCOPEGUARD_H
