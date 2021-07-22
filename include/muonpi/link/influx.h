#ifndef INFLUX_H
#define INFLUX_H

#include "muonpi/global.h"

#include <mutex>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace muonpi::link {

class influx {
public:
    struct tag {
        std::string name;
        std::string field;
    };
    struct field {
        std::string name;
        std::variant<std::string, bool, std::int_fast64_t, double, std::size_t, std::uint8_t, std::uint16_t, std::uint32_t> value;
    };

    struct configuration {
        std::string host {};
        struct Login {
            std::string username {};
            std::string password {};
        } login;
        std::string database {};
    };

    class entry {
    public:
        entry() = delete;

        auto operator<<(const tag& tag) -> entry&;
        auto operator<<(const field& field) -> entry&;

        [[nodiscard]] auto commit(std::int_fast64_t timestamp) -> bool;

    private:
        std::ostringstream m_tags {};
        std::ostringstream m_fields {};

        influx& m_link;

        friend class influx;

        entry(const std::string& measurement, influx& link);
    };

    influx(configuration config);
    influx();
    ~influx();

    [[nodiscard]] auto measurement(const std::string& measurement) -> entry;

private:
    [[nodiscard]] auto send_string(const std::string& query) const -> bool;

    static constexpr short s_port { 8086 };

    std::mutex m_mutex;

    configuration m_config {};
};

}

#endif // INFLUX_H
