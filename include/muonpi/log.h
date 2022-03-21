#ifndef MUONPI_LOG_H
#define MUONPI_LOG_H

#include "muonpi/global.h"

#include <functional>
#include <iostream>
#include <sstream>
#include <string>

namespace muonpi::log {

enum Level : std::uint8_t {
    Shutdown = 0b0000'0001,
    Emergency = Shutdown | 0b0000'0010,
    Alert = Shutdown | 0b0000'0100,
    Critical = Shutdown | 0b0000'0110,
    Error = 0b0001'0000,
    Warning = 0b0010'0000,
    Notice = 0b0011'0000,
    Debug = 0b0100'0000,
    Info = 0b1000'0000
};

/**
 * @brief The system class
 * Initialisation and logging system configuration
 */
class LIBMUONPI_PUBLIC system {
public:
    /**
     * @brief
     * @param l Maximum Log level to show
     */
    static void setup(
        Level l,
        std::function<void(int)> callback = [](int c) { exit(c); },
        std::ostream& str = std::cerr);

    system(Level l, std::function<void(int)> cb, std::ostream& str);

    [[nodiscard]] static auto level() -> Level;
    [[nodiscard]] static auto stream() -> std::ostream&;
    static void callback(int exit_code);

private:
    static std::unique_ptr<system> s_singleton;

    Level m_level {};
    std::function<void(int)> m_callback {};
    std::ostream& m_stream;
};

template <Level L>
class LIBMUONPI_PUBLIC logger {
public:
    template <typename T>
    auto operator<<(T content) -> logger<L>&
    {
        m_stream << content;
        return *this;
    }

    logger(const std::string& component, int exit_code = 0)
        : m_exit_code { exit_code }
    {
        m_stream << to_string();
        if (!component.empty()) {
            m_stream << " (" << component << ')';
        }
        if (m_stream.tellp() > 0) {
            m_stream << ": ";
        }
    }

    logger() = default;

    ~logger()
    {
        if (((L & Level::Info) > 0) || (L <= system::level())) {
            m_stream << '\n';
            system::stream() << m_stream.rdbuf() << std::flush;
        }
        if constexpr ((L & Level::Shutdown) > 0) {
            system::callback(m_exit_code);
        }
    }

private:
    std::stringstream m_stream {};
    int m_exit_code { 0 };

    [[nodiscard]] constexpr static auto to_string() -> const char*
    {
        switch (L) {
        case Level::Debug:
            return "Debug";
        case Level::Info:
            return "";
        case Level::Notice:
            return "Notice";
        case Level::Warning:
            return "Warning";
        case Level::Error:
            return "Error";
        case Level::Critical:
            return "Critical";
        case Level::Alert:
            return "Alert";
        case Level::Emergency:
            return "Emergency";
        }
        return "";
    }
};

[[nodiscard]] auto LIBMUONPI_PUBLIC debug(const std::string& component = {})
    -> logger<Level::Debug>;
[[nodiscard]] auto LIBMUONPI_PUBLIC info(const std::string& component = {}) -> logger<Level::Info>;
[[nodiscard]] auto LIBMUONPI_PUBLIC notice(const std::string& component = {})
    -> logger<Level::Notice>;
[[nodiscard]] auto LIBMUONPI_PUBLIC warning(const std::string& component = {})
    -> logger<Level::Warning>;
[[nodiscard]] auto LIBMUONPI_PUBLIC error(const std::string& component = {})
    -> logger<Level::Error>;
[[nodiscard]] auto LIBMUONPI_PUBLIC critical(int exit_code = 1, const std::string& component = {})
    -> logger<Level::Critical>;
[[nodiscard]] auto LIBMUONPI_PUBLIC alert(int exit_code = 1, const std::string& component = {})
    -> logger<Level::Alert>;
[[nodiscard]] auto LIBMUONPI_PUBLIC emergency(int exit_code = 1, const std::string& component = {})
    -> logger<Level::Emergency>;

} // namespace muonpi::log

#endif // MUONPI_LOG_H
