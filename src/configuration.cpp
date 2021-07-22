#include "muonpi/configuration.h"

namespace muonpi {

std::size_t config::initialisation::s_instances { 0 };


auto config::setup(const std::string& description) -> initialisation
{
    return initialisation{description, m_options};
}

auto config::is_set(const std::string& name) -> bool
{
    return m_options.find(name) != m_options.end();
}



auto config::initialisation::add_option(const std::string& name, const std::string& description) -> initialisation&
{
    m_init(name.c_str(), description.c_str());
    return *this;
}

void config::initialisation::commit(int argc, const char* argv[])
{
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, m_desc), m_options);
    s_instances--;
    if (s_instances == 0) {
        boost::program_options::notify(m_options);
    }
}

void config::initialisation::commit(const std::string& filename)
{
    std::ifstream ifs { filename };
    if (ifs) {
        boost::program_options::store(boost::program_options::parse_config_file(ifs, m_desc), m_options);
    } else {
        log::error() << "Could not open configuration file '" << filename << "'.";
        throw std::runtime_error("Could not open configuration file '" + filename + "'.");
    }
    s_instances--;
    if (s_instances == 0) {
        boost::program_options::notify(m_options);
    }
}

void config::initialisation::print(std::ostream &ostream) const
{
    ostream<<m_desc<<'\n';
}

config::initialisation::initialisation(const std::string& name, boost::program_options::variables_map& options)
    : m_options { options }
    , m_desc { name }
    , m_init { m_desc.add_options() }
{
    s_instances++;
}

} // namespace muonpi
