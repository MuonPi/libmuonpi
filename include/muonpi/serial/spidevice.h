#ifndef MUONPI_SERIAL_SPIDEVICE_H
#define MUONPI_SERIAL_SPIDEVICE_H

#include "muonpi/scopeguard.h"

#include <chrono>
#include <cinttypes> // std::uint8_t, etc
#include <iostream>
#include <string>
#include <vector>
#include <set>

namespace muonpi::serial {

/*
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t mode;
    uint8_t bits_per_word;
    uint32_t speed;
    uint16_t delay;
} spi_config_t;

#ifdef __cplusplus
}
#endif    
*/
#define SPI_CPHA        0x01
#define SPI_CPOL        0x02

#define SPI_MODE_0      (0|0)
#define SPI_MODE_1      (0|SPI_CPHA)
#define SPI_MODE_2      (SPI_CPOL|0)
#define SPI_MODE_3      (SPI_CPOL|SPI_CPHA)

#define SPI_CS_HIGH     0x04
#define SPI_LSB_FIRST       0x08
#define SPI_3WIRE       0x10
#define SPI_LOOP        0x20
#define SPI_NO_CS       0x40
#define SPI_READY       0x80
#define SPI_TX_DUAL     0x100
#define SPI_TX_QUAD     0x200
#define SPI_RX_DUAL     0x400
#define SPI_RX_QUAD     0x800
#define SPI_CS_WORD     0x1000
#define SPI_TX_OCTAL        0x2000
#define SPI_RX_OCTAL        0x4000
#define SPI_3WIRE_HIZ       0x8000
class spi_device {
public:
    enum class Flags : std::uint8_t {
        None = 0,
        Normal = 0x01,
        Force = 0x02,
        Unreachable = 0x04,
        Failed = 0x08,
        Locked = 0x10
    };
    
    typedef std::uint16_t mode_t;
    
    enum class MODE : mode_t {
        CPHA=           0x01,
        CPOL=           0x02,
        MODE0=          0,
        MODE1=          CPHA,
        MODE2=          CPOL,
        MODE3=          CPHA | CPOL,
        CS_HIGH=        0x04,
        LSB_FIRST=      0x08,
        THREE_WIRE=     0x10,
        LOOP=           0x20,
        NO_CS=          0x40,
        READY=          0x80,
        TX_DUAL=        0x100,
        TX_QUAD=        0x200,
        RX_DUAL=        0x400,
        RX_QUAD=        0x800,
        CS_WORD=        0x1000,
        TX_OCTAL=       0x2000,
        RX_OCTAL=       0x4000,
        THREE_WIRE_HIZ= 0x8000
    };

    typedef struct {
        mode_t mode;
        std::uint8_t bits_per_word;
        std::uint32_t clk_rate;
        std::uint16_t delay;
    } config_t;

    explicit spi_device(const std::string& device_path);

    virtual ~spi_device();

    [[nodiscard]] auto is_open() const -> bool;
    void close() const;
    
    [[nodiscard]] auto set_config( config_t config ) -> bool;
    [[nodiscard]] auto config() const -> config_t;

    [[nodiscard]] virtual auto present() -> bool;
    [[nodiscard]] virtual auto identify() -> bool;

    [[nodiscard]] auto io_errors() const -> std::size_t;

    [[nodiscard]] auto rx_bytes() const -> std::size_t;
    [[nodiscard]] auto tx_bytes() const -> std::size_t;

    [[nodiscard]] auto flag_set(Flags flag) const -> bool;

    void lock(bool locked = true);
    [[nodiscard]] auto locked() const -> bool;

    [[nodiscard]] auto last_access_duration() const -> std::chrono::microseconds;

    void set_name(std::string name);
    [[nodiscard]] auto name() const -> std::string;
    
    void set_clock_rate(unsigned int clk_rate);
    [[nodiscard]] auto clock_rate() const -> unsigned int;

    [[nodiscard]] auto read(std::uint8_t* buffer, std::size_t n_bytes = 1) -> bool;
    [[nodiscard]] auto read(std::uint16_t* buffer, std::size_t n_words = 1) -> bool;

    [[nodiscard]] auto write(std::uint8_t* buffer, std::size_t n_bytes = 1) -> bool;
    [[nodiscard]] auto write(const std::uint16_t* buffer, std::size_t n_words = 1) -> bool;
    
    [[nodiscard]] auto transfer(std::uint8_t* tx_buffer, std::uint8_t* rx_buffer, std::size_t n_bytes = 1) -> bool;
    [[nodiscard]] auto transfer(std::uint16_t* tx_buffer, std::uint16_t* rx_buffer, std::size_t n_words = 1) -> bool;

protected:
    void set_flag(Flags flag);
    void unset_flag(Flags flag);

    void start_timer();
    void stop_timer();

    [[nodiscard]] auto setup_timer() -> scope_guard;

private:

    static constexpr char s_default_device_path[] { "/dev/spidev0.0" };

    std::string m_device_path { s_default_device_path };

    int m_handle {};
    bool m_locked { false };

    std::size_t m_rx_bytes {};
    std::size_t m_tx_bytes {};

    std::size_t m_io_errors {};
    std::chrono::microseconds m_last_duration {}; // the last time measurement's result is stored here

    std::string m_name { "SPI device" };
    std::uint8_t m_flags { };
    config_t m_config { static_cast<mode_t>(MODE::MODE0), 8u, 1'000'000UL, 0 };

    std::chrono::system_clock::time_point m_start {};
};

} // namespace muonpi::serial

#endif // MUONPI_SERIAL_SPIDEVICE_H
