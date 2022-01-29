#include "muonpi/utility.h"
#include "muonpi/log.h"

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <ifaddrs.h>
#include <iomanip>
#include <linux/if_link.h>
#include <netdb.h>
#include <random>
#include <regex>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

namespace muonpi {

message_constructor::message_constructor(char delimiter)
    : m_delimiter { delimiter }
{
}

void message_constructor::add_field(const std::string& field)
{
    if (!m_message.empty()) {
        m_message += m_delimiter;
    }
    m_message += field;
}

auto message_constructor::get_string() const -> std::string
{
    return m_message;
}

message_parser::message_parser(std::string message, char delimiter)
    : m_content { std::move(message) }
    , m_iterator { m_content.begin() }
    , m_delimiter { delimiter }
{
    while (!at_end()) {
        read_field();
    }
}

void message_parser::skip_delimiter()
{
    while ((*m_iterator == m_delimiter) && (m_iterator != m_content.end())) {
        m_iterator++;
    }
}

void message_parser::read_field()
{
    skip_delimiter();
    std::string::iterator start { m_iterator };
    while ((*m_iterator != m_delimiter) && (m_iterator != m_content.end())) {
        m_iterator++;
    }
    std::string::iterator end { m_iterator };

    if (start != end) {
        m_fields.emplace_back(start, end);
    }
}

auto message_parser::at_end() -> bool
{
    skip_delimiter();
    return m_iterator == m_content.end();
}

auto message_parser::size() const -> std::size_t
{
    return m_fields.size();
}

auto message_parser::empty() const -> bool
{
    return m_fields.empty();
}

auto message_parser::operator[](std::size_t i) const -> std::string
{
    if (i >= size()) {
        return std::string{};
    }
    return std::string { m_fields[i].first, m_fields[i].second };
}

auto message_parser::get() const -> std::string
{
    return m_content;
}

constexpr static std::uint64_t lower_bits { 0x00000000FFFFFFFF };
constexpr static std::uint64_t upper_bits { 0xFFFFFFFF00000000 };

guid::guid(std::size_t hash, std::uint64_t time)
    : m_first { get_mac() ^ hash ^ (get_number() & lower_bits) }
    , m_second { time ^ (get_number() & upper_bits) }
{
}

auto guid::to_string() const -> std::string
{
    constexpr static std::size_t number_width { 16 };
    std::ostringstream out {};
    out << std::right << std::hex << std::setfill('0') << std::setw(number_width) << m_first << std::setw(number_width) << m_second;
    return out.str();
}

auto guid::get_mac() -> std::uint64_t
{
    static std::uint64_t addr { 0 };
    if (addr != 0) {
        return addr;
    }

    ifaddrs* ifaddr { nullptr };
    if (getifaddrs(&ifaddr) == -1) {
        log::error() << "Could not get MAC address.";
        return {};
    }

    std::string ifname {};
    for (ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }
        ifname = ifa->ifa_name;
        if (ifname == "lo") {
            continue;
        }
        break;
    }
    freeifaddrs(ifaddr);
    if (ifname.empty()) {
        log::error() << "Could not get MAC address.";
        return {};
    }

    std::ifstream iface("/sys/class/net/" + ifname + "/address");
    std::string str((std::istreambuf_iterator<char>(iface)), std::istreambuf_iterator<char>());
    if (!str.empty()) {
        constexpr static int base { 16 };
        addr = std::stoull(std::regex_replace(str, std::regex(":"), ""), nullptr, base);
    }
    return addr;
}

auto guid::get_number() -> std::uint64_t
{
    static std::mt19937 gen { static_cast<std::size_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) };
    static std::uniform_int_distribution<std::uint64_t> distribution { 0, std::numeric_limits<std::uint64_t>::max() };

    return distribution(gen);
}

namespace Version::libmuonpi {
    auto string() -> std::string
    {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch) + "-" + std::string { additional };
    }
} // namespace Version::libmuonpi
} // namespace muonpi
