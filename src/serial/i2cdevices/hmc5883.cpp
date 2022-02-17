#include "muonpi/serial/i2cdevices/hmc5883.h"

#include "muonpi/scopeguard.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <thread>

namespace muonpi::serial::devices {

/*
 * HMC5883 3 axis magnetic field sensor (Honeywell)
 */

HMC5883::HMC5883(i2c_bus& bus, std::uint8_t address)
    : i2c_device(bus, address) {
    set_name("HMC5883");
    m_addresses_hint = {0x1e};
}

HMC5883::~HMC5883() = default;

auto HMC5883::write_config() -> bool {
    std::uint8_t bitmask =
        static_cast<std::uint8_t>(m_config) | static_cast<std::uint8_t>(m_averages);
    if (write(static_cast<std::uint8_t>(Reg::ConfigA), &bitmask, 1u) != 1)
        return false;
    return true;
}

auto HMC5883::init() -> bool {
    if (!write_config() || !set_gain(m_gain))
        return false;
    return true;
}

auto HMC5883::set_gain(std::uint8_t gain) -> bool {
    if (gain > 0x07)
        return false;
    std::uint8_t bitmask = gain << 5u;
    if (write(static_cast<std::uint8_t>(Reg::ConfigB), &bitmask, 1u) != 1)
        return false;
    m_gain = gain;
    return true;
}

auto HMC5883::getXYZRawValues() -> std::optional<std::vector<int>> {
    std::uint8_t start_cmd {0x01}; // start single measurement
    if (write(static_cast<std::uint8_t>(Reg::Mode), &start_cmd, 1u) != 1) {
        return std::nullopt;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(6));

    std::array<std::uint16_t, 3> data_buffer {0, 0, 0};
    if (read(static_cast<std::uint8_t>(Reg::Data), data_buffer.data(), 3u) != 3) {
        return std::nullopt;
    }

    // generate the x,y,z entries: cast to signed integer and reorder, since the readout sequence is
    // x,z,y
    std::vector<int> values {static_cast<std::int16_t>(data_buffer[Axis::X]),
                             static_cast<std::int16_t>(data_buffer[Axis::Z]),
                             static_cast<std::int16_t>(data_buffer[Axis::Y])};

    if (values[Axis::X] >= -2048 && values[Axis::X] < 2048 && values[Axis::Y] >= -2048
        && values[Axis::Y] < 2048 && values[Axis::Z] >= -2048 && values[Axis::Z] < 2048) {
        return std::optional<std::vector<int>> {std::move(values)};
    }
    return std::nullopt;
}

auto HMC5883::get_magnetic_field_vector() -> std::optional<magnetic_field_t> {
    auto raw_vector = getXYZRawValues();
    if (!raw_vector)
        return std::nullopt;

    magnetic_field_t field_vector;
    std::transform(raw_vector->begin(),
                   raw_vector->end(),
                   std::back_inserter(field_vector),
                   [this](int component) -> double { return s_gains[m_gain] * component / 1000.; });

    return std::optional<magnetic_field_t> {std::move(field_vector)};
}

auto HMC5883::magnitude(const magnetic_field_t& b_field) -> double {
    double square_sum =
        std::accumulate(b_field.cbegin(), b_field.cend(), 0, [](double sum, double axis_value) {
            return sum + axis_value * axis_value;
        });
    return std::sqrt(square_sum);
}

auto HMC5883::identify() -> bool {
    if (flag_set(Flags::Failed)) {
        return false;
    }
    if (!present()) {
        return false;
    }

    std::array<std::uint8_t, 3> id_buffer {0, 0, 0};
    if (read(static_cast<std::uint8_t>(Reg::ID), id_buffer.data(), 3) != 3)
        return false;
    if (id_buffer[0] == 'H' && id_buffer[1] == '4' && id_buffer[2] == '3') {
        return true;
    }
    return false;
}

} // namespace muonpi::serial::devices
