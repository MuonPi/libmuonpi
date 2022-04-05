#ifndef MUONPI_SERIAL_I2CDEVICES_IOEXTENDER_H
#define MUONPI_SERIAL_I2CDEVICES_IOEXTENDER_H
#include "muonpi/serial/i2cbus.h"
#include "muonpi/serial/i2cdevice.h"

#include <optional>

namespace muonpi::serial::devices {

template <std::size_t BITS>
/**
 * @brief I2C io extender device class.
 * This class provides access to i2c 4-bit and 8-bit bidirectional digital i/o
 * extenders.
 * @note valid template specializations are available for values of the template
 * parameter BITS of 4 and 8.
 */
class io_extender : public i2c_device {
public:
    static constexpr std::size_t width() {
        return BITS;
    }
    explicit io_extender(i2c_bus& bus, std::uint8_t address = InvalidI2cAddress);

    /**
     * @brief set direction bit mask
     * @param output_mask bitmask with bits set for output lines
     * @return true on success
     */
    [[nodiscard]] auto set_direction_mask(std::uint8_t output_mask) -> bool;

    /**
     * @brief set states of output lines
     * @param state_mask mask with output line states
     * @return true on success
     */
    [[nodiscard]] auto set_output_states(std::uint8_t state_mask) -> bool;

    /**
     * @brief get states of output lines
     * @return std::optional with output line states
     */
    [[nodiscard]] auto get_output_states() -> std::optional<std::uint8_t>;

    /**
     * @brief get states of input lines
     * @return std::optional with input line states
     */
    [[nodiscard]] auto get_input_states() -> std::optional<std::uint8_t>;

    [[nodiscard]] auto present() -> bool override;
    [[nodiscard]] auto identify() -> bool override;

private:
    static constexpr std::size_t bit_mask {(1 << BITS) - 1};
    enum REG
    {
        INPUT    = 0x00,
        OUTPUT   = 0x01,
        POLARITY = 0x02,
        CONFIG   = 0x03
    };
};

/***************************
 * Implementation part
 ***************************/
template <std::size_t BITS>
io_extender<BITS>::io_extender(i2c_bus& bus, std::uint8_t address)
    : i2c_device(bus, address) {
    static_assert(BITS == 4 || BITS == 8,
                  "trying to instantiate io extender with unsupported bit width");
    set_name("undef io extender");
}

template <>
io_extender<4>::io_extender(i2c_bus& bus, std::uint8_t address)
    : i2c_device(bus, address) {
    set_name("4-bit io extender");
}

template <>
io_extender<8>::io_extender(i2c_bus& bus, std::uint8_t address)
    : i2c_device(bus, address) {
    set_name("8-bit io extender");
}

template <std::size_t BITS>
auto io_extender<BITS>::set_direction_mask(std::uint8_t output_mask) -> bool {
    std::uint8_t data = ~(output_mask & bit_mask);
    scope_guard  timer_guard {setup_timer()};
    if (1 != write(REG::CONFIG, &data, 1)) {
        return false;
    }
    return true;
}

template <std::size_t BITS>
auto io_extender<BITS>::set_output_states(std::uint8_t state_mask) -> bool {
    std::uint8_t data = state_mask & bit_mask;
    scope_guard  timer_guard {setup_timer()};
    if (1 != write(REG::OUTPUT, &data, 1)) {
        return false;
    }
    return true;
}

template <std::size_t BITS>
auto io_extender<BITS>::get_input_states() -> std::optional<std::uint8_t> {
    std::uint8_t inport {0x00};
    scope_guard  timer_guard {setup_timer()};
    if (1 != read(REG::INPUT, &inport, 1)) {
        return std::nullopt;
    }
    return std::optional<std::uint8_t> {inport & bit_mask};
}

template <std::size_t BITS>
auto io_extender<BITS>::get_output_states() -> std::optional<std::uint8_t> {
    std::uint8_t outport {0x00};
    scope_guard  timer_guard {setup_timer()};
    if (1 != read(REG::OUTPUT, &outport, 1)) {
        return std::nullopt;
    }
    return std::optional<std::uint8_t> {outport & bit_mask};
}

template <std::size_t BITS>
auto io_extender<BITS>::present() -> bool {
    std::uint8_t databyte {0x00};
    return (1 == read(&databyte, 1));
}

template <std::size_t BITS>
auto io_extender<BITS>::identify() -> bool {
    if (flag_set(Flags::Failed)) {
        return false;
    }
    if (!present()) {
        return false;
    }

    std::uint8_t bytereg {0};
    if (read(static_cast<std::uint8_t>(REG::INPUT), &bytereg) == 0) {
        return false;
    }
    if ((bytereg & 0xf0) != 0xf0) {
        return false;
    }
    if (read(static_cast<std::uint8_t>(REG::POLARITY), &bytereg) == 0) {
        return false;
    }
    if ((bytereg & 0xf0) != 0x00) {
        return false;
    }

    if (read(static_cast<std::uint8_t>(REG::CONFIG), &bytereg) == 0) {
        return false;
    }
    if ((bytereg & 0xf0) != 0xf0) {
        return false;
    }
    if (read(static_cast<std::uint8_t>(0x04), &bytereg, 1) > 0) {
        return false;
    }
    return true;
}

} // namespace muonpi::serial::devices
#endif // MUONPI_SERIAL_I2CDEVICES_IOEXTENDER_H
