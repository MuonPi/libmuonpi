#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "muonpi/global.h"
#include "muonpi/log.h"

#include <boost/program_options.hpp>

#include <fstream>

namespace muonpi {

class config {
public:
    class initialisation {
    public:
        template <typename T>
        auto add_option(const std::string& name, T value) -> initialisation&;

        template <typename T>
        auto add_option(const std::string& name, T value, const std::string& description) -> initialisation&;

        auto add_option(const std::string& name, const std::string& description) -> initialisation&;

        void commit(int argc, const char* argv[]);

        void commit(const std::string& filename);

        void print(std::ostream& ostream = std::cout) const;

    private:
        initialisation(const std::string& name, boost::program_options::variables_map& options);

        friend class config;

        boost::program_options::variables_map& m_options;
        boost::program_options::options_description m_desc;
        boost::program_options::options_description_easy_init m_init;
        static std::size_t s_instances;
    };

    [[nodiscard]] auto setup(const std::string& description) -> initialisation;

    template <typename T>
    [[nodiscard]] auto get(std::string name) -> T;

    [[nodiscard]] auto is_set(const std::string& name) -> bool;

private:
    boost::program_options::variables_map m_options {};
};


template <typename T>
auto config::get(std::string name) -> T
{
    if (!is_set(name)) {
        log::error()<<"Option '" << name << "' not set.";
        throw std::runtime_error("Option '" + name + "' not set.");
    }
    return m_options[std::move(name)].as<T>();
}

template <typename T>
auto config::initialisation::add_option(const std::string& name, T value) -> initialisation&
{
    m_init(name.c_str(), value);
    return *this;
}

template <typename T>
auto config::initialisation::add_option(const std::string& name, T value, const std::string& description) -> initialisation&
{
    m_init(name.c_str(), value, description.c_str());
    return *this;
}

auto operator<<(std::ostream& ostream, const config::initialisation& initialisation) -> std::ostream&
{
    initialisation.print(ostream);
    return ostream;
}

}

#endif // CONFIGURATION_H
