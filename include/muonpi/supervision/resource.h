#ifndef RESOURCETRACKER_H
#define RESOURCETRACKER_H

#include "muonpi/global.h"

#include <cstdint>

namespace muonpi::supervision {

class LIBMUONPI_PUBLIC resource {
public:
    struct data_t {
        float process_cpu_load {};
        float system_cpu_load {};
        float memory_usage {};
    };

    /**
     * @brief get_data Gets the data about used system resources
     * @return The acquired data
     */
    [[nodiscard]] auto get_data() -> data_t;

private:
    struct {
        std::uint64_t total_time_last {};
        std::uint64_t process_time_last {};
        std::uint64_t system_time_last {};
    } m_cpu {};

    bool m_first { true };
};
}

#endif // RESOURCETRACKER_H
