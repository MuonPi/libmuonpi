#include "muonpi/configuration.h"

namespace muonpi {

const std::unique_ptr<config> config::s_singleton { std::make_unique<config>() };
std::size_t config::initialisation::s_instances { 0 };


auto config::setup(std::string description) -> initialisation
{
    return initialisation{std::move(description), s_singleton->m_options};
}

auto config::is_set(const std::string& name) -> bool
{
    return s_singleton->m_options.find(name) != s_singleton->m_options.end();
}

void config::print_help()
{
    for (auto& desc: s_singleton->m_descriptions) {
        std::cout<<desc<<'\n';
    }
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

config::initialisation::initialisation(std::string name, boost::program_options::variables_map& options)
    : m_options { options }
    , m_desc { std::move(name) }
    , m_init { m_desc.add_options() }
{
    s_instances++;
    config::s_singleton->m_descriptions.emplace_back(m_desc);
}

} // namespace muonpi
