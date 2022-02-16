#ifndef MUONPI_UTILITY_H
#define MUONPI_UTILITY_H

#include "muonpi/global.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace muonpi {

class LIBMUONPI_PUBLIC message_constructor {
public:
    /**
     * @brief MessageConstructor
     * @param delimiter The delimiter which separates the fields
     */
    explicit message_constructor(char delimiter);

    /**
     * @brief add_field Adds a field to the complete message
     * @param field The field to add
     */
    void add_field(const std::string& field);

    /**
     * @brief get_string Gets the complete string
     * @return The completed string
     */
    [[nodiscard]] auto get_string() const -> std::string;

private:
    std::string m_message {};
    char        m_delimiter;
};

class LIBMUONPI_PUBLIC message_parser {
public:
    /**
     * @brief MessageParser
     * @param message The message to parse
     * @param delimiter The delimiter separating the fields in the message
     */
    message_parser(std::string message, char delimiter);

    /**
     * @brief size
     * @return The number of fields in the message
     */
    [[nodiscard]] auto size() const -> std::size_t;
    /**
     * @brief empty
     * @return true if there are no fields
     */
    [[nodiscard]] auto empty() const -> bool;

    /**
     * @brief operator [] Access one field in the message
     * @param i The index of the field
     * @return The string contained in the field, an empty string for invalid indices
     */
    [[nodiscard]] auto operator[](std::size_t i) const -> std::string;

    /**
     * @brief get Get the original string
     * @return The original string
     */
    [[nodiscard]] auto get() const -> std::string;

private:
    /**
     * @brief skip_delimiter Skips all delimiters until the next field
     */
    void skip_delimiter();
    /**
     * @brief read_field reads the next field
     */
    void read_field();
    /**
     * @brief at_end
     * @return True if the iterator is at the end of the string
     */
    [[nodiscard]] auto at_end() -> bool;

    std::string           m_content {};
    std::string::iterator m_iterator;
    char                  m_delimiter {};

    std::vector<std::pair<std::string::iterator, std::string::iterator>> m_fields {};
};

class LIBMUONPI_PUBLIC guid {
public:
    guid(std::size_t hash, std::uint64_t time);

    [[nodiscard]] auto        to_string() const -> std::string;
    [[nodiscard]] static auto get_mac() -> std::uint64_t;

private:
    [[nodiscard]] static auto get_number() -> std::uint64_t;

    std::uint64_t m_first {0};
    std::uint64_t m_second {0};
};

} // namespace muonpi
#endif // MUONPI_UTILITY_H
