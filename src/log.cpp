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

auto debug(const std::string& component) -> logger<Level::Debug>
{
    return logger<Level::Debug>{ component };
}

auto info(const std::string& component) -> logger<Level::Info>
{
    return logger<Level::Info>{ component };
}

auto notice(const std::string& component) -> logger<Level::Notice>
{
    return logger<Level::Notice>{ component };
}

auto warning(const std::string& component) -> logger<Level::Warning>
{
    return logger<Level::Warning>{ component };
}

auto error(const std::string& component) -> logger<Level::Error>
{
    return logger<Level::Error>{ component };
}

auto critical(int exit_code, const std::string& component) -> logger<Level::Critical>
{
    return logger<Level::Critical>{ component, exit_code };
}

auto alert(int exit_code, const std::string& component) -> logger<Level::Alert>
{
    return logger<Level::Alert>{ component, exit_code };
}

auto emergency(int exit_code, const std::string& component) -> logger<Level::Emergency>
{
    return logger<Level::Emergency>{ component, exit_code };
}

} // namespace muonpi::log
