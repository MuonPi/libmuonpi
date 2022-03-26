#include <cstdint>
#include <iomanip>
#include <iostream>
#include <muonpi/log.h>
#include <muonpi/serial/i2cbus.h>
#include <muonpi/serial/i2cdevices.h>
#include <sstream>

using namespace muonpi;

auto main() -> int {
    log::system::setup(
        muonpi::log::Level::Info,
        [](int c) { exit(c); },
        std::cerr);

    serial::i2c_bus bus {"/dev/i2c-1"};
    if (!bus.general_call.reset()) {
        log::error() << "resetting bus through general call command";
    }

    log::info() << "scanning bus " << bus.address() << " for devices...";

    for (std::uint8_t addr = 4; addr < 0x7c; ++addr) {
        auto& dev = bus.open<serial::i2c_device>(addr);

        if (dev.is_open() && dev.present()) {
            log::info() << "found " << dev.name() << " at 0x" << std::hex << std::setw(2)
                        << std::setfill('0') << static_cast<int>(addr);
        } else {
            bus.close(addr);
        }
    }

    bool                   ok {false};
    constexpr std::uint8_t tempsens_addr {0x4b};
    ok = bus.identify_device<serial::devices::MIC184>(tempsens_addr);
    if (ok) {
        // found the specific device at the expected position, so close the previously created
        // generic i2c_device and reopen as temp sensor device
        bus.close(tempsens_addr);
        auto& tempsensor = bus.open<serial::devices::MIC184>(tempsens_addr);
        log::info() << "identified " << tempsensor.name() << " at 0x" << std::hex << std::setw(2)
                    << std::setfill('0') << static_cast<int>(tempsens_addr)
                    << " : temp=" << std::dec << tempsensor.get_temperature();
    } else {
        log::error() << "error identifying MIC184 at 0x" << std::hex << std::setw(2)
                     << std::setfill('0') << static_cast<int>(tempsens_addr);
    }

    constexpr std::uint8_t adc_addr {0x48};

    ok = bus.identify_device<serial::devices::ADS1115>(adc_addr);
    if (ok) {
        // found the specific device at the expected position, so close the previously created
        // generic i2c_device and reopen as adc device
        bus.close(adc_addr);
        auto& adc = bus.open<serial::devices::ADS1115>(adc_addr);
        log::info() << "identified " << adc.name() << " at 0x" << std::hex << std::setw(2)
                    << std::setfill('0') << static_cast<int>(adc_addr) << " : ch0=" << std::dec
                    << adc.getVoltage(0) << " ch1=" << std::dec << adc.getVoltage(1)
                    << " ch2=" << std::dec << adc.getVoltage(2) << " ch3=" << std::dec
                    << adc.getVoltage(3)
                    << "; ro-time=" << 1e-3 * adc.last_access_duration().count() << "ms";
    } else {
        log::error() << "error identifying ADS1115 at 0x" << std::hex << std::setw(2)
                     << std::setfill('0') << static_cast<int>(adc_addr);
    }

    constexpr std::uint8_t i2c_extender_addr {0x41};
    if (bus.identify_device<serial::devices::PCA9536>(i2c_extender_addr)) {
        // found the specific device at the expected position, so close the previously created
        // generic i2c_device and reopen as PCA9536 device
        bus.close(i2c_extender_addr);
        auto& pca = bus.open<serial::devices::PCA9536>(i2c_extender_addr);
        auto  input_state {pca.get_input_states()};
        auto  output_state {pca.get_output_states()};
        if (!input_state && !output_state) {
            log::error() << "reading " << pca.name() << " state registers";
        } else {
            log::info() << "identified " << pca.name() << " at 0x" << std::hex << std::setw(2)
                        << std::setfill('0') << static_cast<int>(i2c_extender_addr)
                        << " : inputs=0x" << std::setw(1) << static_cast<int>(input_state.value())
                        << " : outputs=0x" << std::setw(1) << static_cast<int>(output_state.value())
                        << std::dec;
        }
    } else {
        log::error() << "error identifying PCA9536 at 0x" << std::hex << std::setw(2)
                     << std::setfill('0') << static_cast<int>(i2c_extender_addr);
    }

    constexpr std::uint8_t eeprom_addr {0x50};
    if (bus.identify_device<serial::devices::eeproms::MC24AA02UID>(eeprom_addr)) {
        // found the specific device at the expected position, so close the previously created
        // generic i2c_device and reopen as eeprom device
        bus.close(eeprom_addr);
        auto&                 eep   = bus.open<serial::devices::eeproms::MC24AA02UID>(eeprom_addr);
        [[maybe_unused]] bool id_ok = eep.identify();
        log::info() << "identified " << eep.name() << " at 0x" << std::hex << std::setw(2)
                    << std::setfill('0') << static_cast<int>(eeprom_addr);
        log::info() << "EEPROM content:";
        std::size_t page {0};
        while (page < eep.size() / eep.page_size()) {
            std::ostringstream ostr;
            ostr << std::hex << std::setw((eep.size() <= 0x100) ? 2 : 4) << std::setfill('0')
                 << page * eep.page_size() << ':';
            std::array<std::uint8_t, eep.page_size()> page_buffer {};
            if (eep.read(page * eep.page_size(), page_buffer.data(), eep.page_size())
                != eep.page_size()) {
                log::error() << "reading eeprom content";
            }
            for (std::size_t page_addr {0}; page_addr < eep.page_size(); page_addr++) {
                ostr << ' ' << std::hex << std::setw(2) << std::setfill('0')
                     << static_cast<std::uint16_t>(page_buffer.at(page_addr));
            }
            log::info() << ostr.str();
            page++;
        }
        log::info() << "eeprom read duration: " << eep.last_access_duration().count() << "us";

    } else {
        log::error() << "error identifying EEPROM at 0x" << std::hex << std::setw(2)
                     << std::setfill('0') << static_cast<int>(eeprom_addr);
    }

    log::info() << "nr of instantiated devices: " << bus.count_devices();

    // close all devices previously found
    while (bus.count_devices() != 0U) {
        auto addr = bus.get_devices().begin()->first;
        bus.close(addr);
    }
    log::debug() << "nr of rx bytes: " << bus.rx_bytes();
    log::debug() << "nr of tx bytes: " << bus.tx_bytes();
}
