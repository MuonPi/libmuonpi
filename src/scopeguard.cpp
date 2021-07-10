#include "muonpi/scopeguard.h"

namespace muonpi {

scope_guard::scope_guard(scope_guard&& other) noexcept
    : m_cleanup { other.dissolve() }
{
    other.dismiss();
}

scope_guard::~scope_guard()
{
    m_cleanup();
}

void scope_guard::dismiss()
{
    m_cleanup = [] {};
}

auto scope_guard::dissolve() -> std::function<void()>
{
    return std::move(m_cleanup);
}

} // namespace muonpi
