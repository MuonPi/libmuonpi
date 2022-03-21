#include "muonpi/link/influx.h"

#include "muonpi/http_request.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace muonpi::link {

influx::entry::entry(const std::string& measurement, influx& link)
    : m_link { link }
{
    m_tags << measurement;
}

auto influx::entry::operator<<(const tag& tag) -> entry&
{
    m_tags << ',' << tag.name << '=' << tag.value;
    return *this;
}

auto influx::entry::commit(std::int_fast64_t timestamp) -> bool
{
    if (m_fields.str().empty()) {
        return false;
    }
    m_tags << ' ' << m_fields.str().substr(1) << ' ' << timestamp;
    return m_link.send_string(m_tags.str());
}

influx::influx(configuration config)
    : m_config { std::move(config) }
{
}

influx::influx() = default;

influx::~influx() = default;

auto influx::measurement(const std::string& measurement) -> entry
{
    return entry { measurement, *this };
}

auto influx::send_string(const std::string& query) const -> bool
{
    http::destination_t destination {};

    destination.host = m_config.host;
    destination.method = http::http_verb::post;
    destination.port = s_port;

    std::ostringstream target {};
    target << "/write?db=" << m_config.database << "&u=" << m_config.login.username
           << "&p=" << m_config.login.password << "&epoch=ms";

    destination.target = target.str();

    std::vector<http::field_t> fields {
        { http::http_field::content_type, "application/x-www-form-urlencoded" },
        { http::http_field::accept, "*/*" }
    };

    auto res = http::http_request(destination, query, false, fields);

    if (res.result() != http::http_status::no_content) {
        log::warning() << "Couldn't write to database: "
                       << std::to_string(static_cast<unsigned>(res.result())) << ": " << res.body();
        return false;
    }
    return true;
}

} // namespace muonpi::link
