#include "muonpi/scopeguard.h"

namespace muonpi {
scope_guard::scope_guard(std::function<void()> cleanup)
    : m_cleanup {std::move(cleanup)} {}

scope_guard::scope_guard(std::function<void()> cleanup, std::function<void()> dismiss)
    : m_cleanup {std::move(cleanup)}
    , m_dismiss {std::move(dismiss)} {}

scope_guard::scope_guard(scope_guard&& other) noexcept
    : m_cleanup {other.dissolve()} {
    other.dismiss();
}

scope_guard::~scope_guard() {
    m_cleanup();
}

void scope_guard::dismiss() {
    m_cleanup = std::move(m_dismiss);
}

auto scope_guard::dissolve() -> std::function<void()> {
    return std::move(m_cleanup);
}

[[nodiscard]] auto wait_for(std::condition_variable&  cv,
                            std::chrono::milliseconds interval,
                            std::chrono::seconds      total_wait) -> bool {
    std::mutex                   mx {};
    std::unique_lock<std::mutex> lock {mx};
    int                          n {1};
    while (cv.wait_for(lock, interval) == std::cv_status::timeout) {
        if (n * interval >= total_wait) {
            return false;
        }
        n++;
    }
    return true;
}

} // namespace muonpi
