#include "muonpi/supervision/resource.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <unistd.h>

namespace muonpi::supervision {

auto resource::get_data() -> data_t {
    std::ifstream total_stream("/proc/stat", std::ios_base::in);

    std::size_t cpu_total {};
    std::size_t system_user {};
    std::size_t system_nice {};
    std::size_t system_system {};

    std::string cpu;

    total_stream >> cpu >> system_user >> system_nice >> system_system;

    cpu_total += system_user + system_nice + system_system;

    constexpr static std::size_t n_data {7};

    for (std::size_t i {0}; i < n_data; i++) {
        std::size_t v {};
        total_stream >> v;
        cpu_total += v;
    }

    total_stream.close();

    std::ifstream stat_stream("/proc/self/stat", std::ios_base::in);

    std::string pid;
    std::string comm;
    std::string state;
    std::string ppid;
    std::string pgrp;
    std::string session;
    std::string tty_nr;
    std::string tpgid;
    std::string flags;
    std::string minflt;
    std::string cminflt;
    std::string majflt;
    std::string cmajflt;
    std::string utime;
    std::string stime;
    std::string cutime;
    std::string cstime;
    std::string priority;
    std::string nice;
    std::string O;
    std::string itrealvalue;
    std::string starttime;

    std::size_t process_user {0};
    std::size_t process_system {0};

    std::size_t vsize {0};
    std::size_t rss {0};

    stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags
        >> minflt >> cminflt >> majflt >> cmajflt >> process_user >> process_system >> cutime
        >> cstime >> priority >> nice >> O >> itrealvalue >> starttime >> vsize >> rss;
    stat_stream.close();

    long page_size_b = sysconf(_SC_PAGE_SIZE);

    auto total   = static_cast<float>(cpu_total - m_cpu.total_time_last);
    auto process = static_cast<float>(process_user + process_system - m_cpu.process_time_last);
    auto system =
        static_cast<float>(system_user + system_system + system_nice - m_cpu.system_time_last);
    m_cpu.total_time_last   = cpu_total;
    m_cpu.process_time_last = process_user + process_system;
    m_cpu.system_time_last  = system_user + system_system + system_nice;

    data_t data;
    data.memory_usage     = static_cast<float>(rss * page_size_b);
    data.process_cpu_load = 0;
    data.system_cpu_load  = 0;

    if (!m_first) {
        data.process_cpu_load = 100.0F * process / std::max(total, 1.0F);
        data.system_cpu_load  = 100.0F * system / std::max(total, 1.0F);
    } else {
        m_first = false;
    }

    return data;
}

} // namespace muonpi::supervision
