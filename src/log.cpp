#include "muonpi/log.h"

#include <memory>
#include <utility>

namespace muonpi::log {

std::unique_ptr<system> system::s_singleton { std::make_unique<system>() };

writer::writer(Level l, std::ostream& stream)
    : m_level { l }
    , m_stream { stream } {}

auto writer::stream() const -> std::ostream& {
    return m_stream;
}

auto writer::level() const -> Level {
    return m_level;
}

void system::set_callback(std::function<void(int)> callback) {
    s_singleton->m_callback = std::move(callback);
}

void system::add_writer(writer w) {
    s_singleton->m_writers.emplace_back(w);
}

void system::setup(Level l, std::function<void(int)> callback, std::ostream& str) {
    s_singleton->m_writers.emplace_back(writer{l, str});
    set_callback(std::move(callback));
}

auto debug(const std::string& component) -> logger<Level::Debug> {
    return logger<Level::Debug> {component};
}

auto info(const std::string& component) -> logger<Level::Info> {
    return logger<Level::Info> {component};
}

auto notice(const std::string& component) -> logger<Level::Notice> {
    return logger<Level::Notice> {component};
}

auto warning(const std::string& component) -> logger<Level::Warning> {
    return logger<Level::Warning> {component};
}

auto error(const std::string& component) -> logger<Level::Error> {
    return logger<Level::Error> {component};
}

auto critical(int exit_code, const std::string& component) -> logger<Level::Critical> {
    return logger<Level::Critical> {component, exit_code};
}

auto alert(int exit_code, const std::string& component) -> logger<Level::Alert> {
    return logger<Level::Alert> {component, exit_code};
}

auto emergency(int exit_code, const std::string& component) -> logger<Level::Emergency> {
    return logger<Level::Emergency> {component, exit_code};
}

} // namespace muonpi::log
