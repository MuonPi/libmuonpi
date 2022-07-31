#ifndef MUONPI_SERIAL_I2CDEVICES_HMC5883_H
#define MUONPI_SERIAL_I2CDEVICES_HMC5883_H

#include "muonpi/serial/i2cdefinitions.h"
#include "muonpi/serial/i2cdevice.h"
#include "muonpi/fixedaddress.h"

namespace muonpi::serial::device {

class hmc5883 : public i2c_device {
public:
    constexpr static fixed_address addresses { 0b0011110 };


    struct configuration_r : public simple_register<std::uint16_t, std::uint8_t, 0x00> {
        constexpr static i2c_tag_type register_tag {i2c_register_tag::read_write};

        const value_type CRA : 1 { 0 };
        value_type MA : 2 { 0b11 };
        value_type DO : 3 { 0b100 };
        value_type MS : 2 { 0 };
        value_type GN : 3 { 0b001 };
        const value_type CRB : 5 { 0 };
    };

    struct mode_r : public simple_register<std::uint8_t, std::uint8_t, 0x02> {
        constexpr static i2c_tag_type register_tag {i2c_register_tag::read_write};

        const value_type MR : 6 { 0 };
        value_type MD : 2 { 0b01 };
    };

    struct data_r : public multi_register<std::int16_t, 3, std::uint8_t, 0x03> {

        value_type X { 0 };
        value_type Y { 0 };
        value_type Z { 0 };

        [[nodiscard]] constexpr auto get() const noexcept -> std::array<value_type, register_length> override {
            return {X, Y, Z};
        }

        constexpr explicit data_r(const std::array<value_type, register_length>& data) noexcept
            : X { data.at(0) }
            , Y { data.at(1) }
            , Z { data.at(2) }
        {}

        constexpr explicit data_r() noexcept = default;
    };

    struct status_r : public simple_register<std::uint8_t, std::uint8_t, 0x09> {

        const value_type SR : 6 {};
        value_type LOCK : 1 {};
        value_type RDY : 1 {};
    };

    struct identification_r : public multi_register<std::uint8_t, 3, std::uint8_t, 0x10> {

        const std::array<value_type, register_length> value { 'H', '4', '3' };

        [[nodiscard]] constexpr auto get() const noexcept -> std::array<value_type, register_length> override {
            return value;
        }

        constexpr explicit identification_r(const std::array<value_type, register_length>& data) noexcept
            : value { data }
        {}

        constexpr explicit identification_r() noexcept = default;
    };
};

} // namespace muonpi::serial::device


#endif
