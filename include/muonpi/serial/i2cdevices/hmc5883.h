#ifndef MUONPI_SERIAL_I2CDEVICES_HMC5883_H
#define MUONPI_SERIAL_I2CDEVICES_HMC5883_H
#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"

#include <cstdint>
#include <optional>
#include <vector>

namespace muonpi::serial::devices {

/* HMC5883 3-axis magnetic field sensor */
class HMC5883
    : public i2c_device
    , public static_device_base<HMC5883> {
public:
    typedef std::vector<float> magnetic_field_t;
    enum Axis
    {
        X = 0,
        Y = 1,
        Z = 2
    };
    enum averages_t : std::uint8_t
    {
        Avg1 = 0x00u,
        Avg2 = 0x20u,
        Avg4 = 0x40u,
        Avg8 = 0x60u
    };
    enum config_t
    {
        Normal       = 0,
        PositiveBias = 1,
        NegativeBias = 2
    };

    explicit HMC5883(i2c_bus& bus, std::uint8_t address = InvalidI2cAddress);
    ~HMC5883() override;

    [[nodiscard]] auto init() -> bool;

    [[nodiscard]] auto get_magnetic_field_vector() -> std::optional<magnetic_field_t>;

    [[nodiscard]] static auto magnitude(const magnetic_field_t& b_field) -> double;

    [[nodiscard]] auto identify() -> bool override;

private:
    bool         m_calibration_valid {false};
    std::uint8_t m_gain {1};
    averages_t   m_averages {averages_t::Avg8};
    config_t     m_config {config_t::Normal};

    // Resolution for the 8 gain settings in mG/LSB
    static constexpr std::array<double, 8> s_gains {0.73, 0.92, 1.22, 1.52, 2.27, 2.56, 3.03, 4.35};
    enum Reg : std::uint8_t
    {
        Config    = 0x00,
        ConfigA   = 0x00,
        ConfigB   = 0x01,
        Mode      = 0x02,
        Data      = 0x03,
        DataX     = 0x03,
        DataX_MSB = 0x03,
        DataX_LSB = 0x04,
        DataZ     = 0x05,
        DataZ_MSB = 0x05,
        DataZ_LSB = 0x06,
        DataY     = 0x07,
        DataY_MSB = 0x07,
        DataY_LSB = 0x08,
        Status    = 0x09,
        ID        = 0x0a,
        IDA       = 0x0a,
        IDB       = 0x0b,
        IDC       = 0x0c
    };

    [[nodiscard]] auto write_config() -> bool;
    [[nodiscard]] auto set_gain(std::uint8_t gain) -> bool;
    uint8_t            readGain();
    [[nodiscard]] auto getXYZRawValues() -> std::optional<std::vector<int>>;
    bool               getXYZMagneticFields(double& x, double& y, double& z);
    bool               readRDYBit();
    bool               readLockBit();
    bool               calibrate(int& x, int& y, int& z);
};

} // namespace muonpi::serial::devices

#endif // MUONPI_SERIAL_I2CDEVICES_HMC5883_H
