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
        std::string value;
    };
    template <typename T>
    struct field {
        std::string name;
        T value;
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

        template <typename T>
        auto operator<<(const field<T>& field) -> entry&
        {
            m_fields << ',' << field.name << "=";
            if constexpr (std::is_same_v<T, std::string>) {
                m_fields << '"' << field.value << '"';
            } else if constexpr (std::is_same_v<T, bool>) {
                m_fields << (field.value?'t':'f');
            } else if constexpr (std::is_floating_point_v<T>) {
                m_fields << field.value;
            } else if constexpr (std::is_integral_v<T>) {
                if constexpr (sizeof(T) < 2) {
                    m_fields << static_cast<std::int32_t>(field.value) << 'i';
                } else {
                    m_fields << field.value << 'i';
                }
            }
            return *this;
        }

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
