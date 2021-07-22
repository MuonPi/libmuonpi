#include <utility>

#include "muonpi/log.h"

namespace muonpi::log {

std::unique_ptr<system> system::s_singleton { nullptr };

system::system(Level l, std::function<void(int)> cb, std::ostream& str)
    : m_level { l }
    , m_callback { std::move(cb) }
    , m_stream { str }
{
}

void system::setup(Level l, std::function<void(int)> callback, std::ostream& str)
{
    if (s_singleton != nullptr) {
        throw std::runtime_error("Double initialsation of logging system");
    }
    s_singleton = std::make_unique<system>(l, std::move(callback), str);
}

auto system::level() -> Level
{
    if (s_singleton == nullptr) {
        throw std::runtime_error("Logging system not initialised");
    }

    return s_singleton->m_level;
}

auto system::stream() -> std::ostream&
{
    if (s_singleton == nullptr) {
        throw std::runtime_error("Logging system not initialised");
    }

    return s_singleton->m_stream;
}

void system::callback(int exit_code)
{
    if (s_singleton == nullptr) {
        throw std::runtime_error("Logging system not initialised");
    }

    s_singleton->m_callback(exit_code);
}

auto debug() -> logger<Level::Debug>
{
    return {};
}

auto info() -> logger<Level::Info>
{
    return {};
}

auto notice() -> logger<Level::Notice>
{
    return {};
}

auto warning() -> logger<Level::Warning>
{
    return {};
}

auto error() -> logger<Level::Error>
{
    return {};
}

auto critical(int exit_code) -> logger<Level::Critical>
{
    return { exit_code };
}

auto alert(int exit_code) -> logger<Level::Alert>
{
    return { exit_code };
}

auto emergency(int exit_code) -> logger<Level::Emergency>
{
    return { exit_code };
}

} // namespace muonpi::log
