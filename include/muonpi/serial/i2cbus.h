#ifndef MUONPI_I2CINTERFACE_H
#define MUONPI_I2CINTERFACE_H

#include "muonpi/serial/i2cdevice.h"

#include <fcntl.h> // open
#include <sys/ioctl.h> // ioctl
#include <sys/time.h> // for gettimeofday()
#include <unistd.h>
#include <stdio.h>

#include <cinttypes>

#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <type_traits>
#include <memory>


namespace muonpi::serial {

class i2c_bus {

public:
    friend class i2c_device;
	
	explicit i2c_bus(std::string address);
    explicit i2c_bus();

    virtual ~i2c_bus();

    [[nodiscard]] auto address() const -> std::string;

    template <typename T, std::enable_if_t<std::is_base_of<i2c_device, T>::value, bool> = true>
    [[nodiscard]] auto open(std::uint8_t address) -> T&
    {
        m_devices.emplace( address, std::make_shared<T>(*this, address) );

        return dynamic_cast<T&>( *( m_devices[address].get() ) );
    }

    template <typename T, std::enable_if_t<std::is_base_of<i2c_device, T>::value, bool> = true>
    [[nodiscard]] auto get(std::uint8_t address) -> T&
    {
        return dynamic_cast<T&>( *( m_devices[address].get() ) );
    }
    
    template <typename T, std::enable_if_t<std::is_base_of<i2c_device, T>::value, bool> = true>
    [[nodiscard]] auto identify_device( std::uint8_t address ) -> bool;

    [[nodiscard]] auto is_open(std::uint8_t address) const -> bool;

    auto close(std::uint8_t address) -> bool;

    [[nodiscard]] auto count_devices() const -> std::size_t;

    [[nodiscard]] auto rx_bytes() const -> std::size_t;
    [[nodiscard]] auto tx_bytes() const -> std::size_t;

    [[nodiscard]] auto get_devices() const -> const std::map<std::uint8_t, std::shared_ptr<i2c_device>>&;

protected:
    std::string m_address { "/dev/i2c-1" };
    std::map<std::uint8_t, std::shared_ptr<i2c_device>> m_devices {};

    std::size_t m_rx_bytes {};
    std::size_t m_tx_bytes {};
};


template <typename T, std::enable_if_t<std::is_base_of<i2c_device, T>::value, bool> = true>
auto i2c_bus::identify_device( std::uint8_t address ) -> bool 
{
	T dev { *this, address };
	if ( !dev.is_open() || !dev.present() ) return false;
	return dev.identify();
}

} // namespace muonpi::serial

#endif // MUONPI_I2CINTERFACE_H
