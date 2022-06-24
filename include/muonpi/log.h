#ifndef MUONPI_LOG_H
#define MUONPI_LOG_H

#include "muonpi/global.h"

#include <functional>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace muonpi::log {

enum Level : std::uint8_t
{
    Shutdown  = 0b0000'0001,
    Emergency = Shutdown | 0b0000'0010,
    Alert     = Shutdown | 0b0000'0100,
    Critical  = Shutdown | 0b0000'0110,
    Error     = 0b0001'0000,
    Warning   = 0b0010'0000,
    Notice    = 0b0011'0000,
    Debug     = 0b0100'0000,
    Info      = 0b1000'0000,
    Invalid   = 0
};

class LIBMUONPI_PUBLIC writer {
public:
    explicit writer(Level l, std::ostream& stream = std::cerr);

    [[nodiscard]] auto stream() const -> std::ostream&;
    [[nodiscard]] auto level() const -> Level;

private:
    Level           m_level;
    std::ostream&   m_stream;
};

/**
 * @brief The system class
 * Initialisation and logging system configuration
 */
class LIBMUONPI_PUBLIC system {
public:
    /**
     * @brief
     * @param l Maximum Log level to show in the default writer.
     * @param callback The callback which gets called from a log message which has the shutdown flag enabled.
     * @param str The stream which gets used by the default writer.
     */
    static void setup(
        Level                    l,
        std::function<void(int)> callback = [](int c) { exit(c); },
        std::ostream&            str      = std::cerr);

    template <Level L>
    /**
     * @brief write a log message.
     * 
     * @param stream The message construction stream.
     * @param exit_code The exit code which to use in case of a set shutdown flag.
     */
    static void write(std::stringstream stream, int exit_code) {
        stream << '\n';
        for (const auto& writer : s_singleton->m_writers) {
            if (((L & Level::Info) > 0) || (L <= writer.level())) {
                writer.stream() << stream.rdbuf() << std::flush;
            }
        }
        if constexpr ((L & Level::Shutdown) > 0) {
            s_singleton->m_callback(exit_code);
        }
    }

    /**
     * @brief Add another writer to the logging system.
     * 
     * @param w The writer to add.
     */
    static void add_writer(writer w);

    /**
    * @brief Set the callback object which gets called in case of a set Shutdown flag.
    * 
    * @param callback The callback object.
    */
    static void set_callback(std::function<void(int)> callback);
private:
    static std::unique_ptr<system> s_singleton;

    std::function<void(int)>    m_callback {[](int c){exit(c);}};
    std::vector<writer>         m_writers{};
};

template <Level L>
class LIBMUONPI_PUBLIC logger {
public:
    template <typename T>
    auto operator<<(T content) -> logger<L>& {
        m_stream << content;
        return *this;
    }

    logger(const std::string& component, int exit_code = 0)
        : m_exit_code {exit_code} {
        m_stream << to_string();
        if (!component.empty()) {
            m_stream << " (" << component << ')';
        }
        if (m_stream.tellp() > 0) {
            m_stream << ": ";
        }
    }

    logger() = default;

    ~logger() {
        system::write<L>(std::move(m_stream), m_exit_code);
    }

private:
    std::stringstream m_stream {};
    int               m_exit_code {0};

    [[nodiscard]] constexpr static auto to_string() -> const char* {
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
