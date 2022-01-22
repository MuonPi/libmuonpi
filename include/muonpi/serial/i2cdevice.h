#ifndef MUONPI_I2CDEVICE_H
#define MUONPI_I2CDEVICE_H


#include <fcntl.h> // open
#include <inttypes.h> // std::uint8_t, etc
#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/ioctl.h> // ioctl
#include <sys/time.h> // for gettimeofday()
#include <unistd.h>
#include <vector>
#include <chrono>



#define DEFAULT_DEBUG_LEVEL 0

namespace muonpi::serial {

class i2c_bus;

// base class fragment static_device_base which implemets static functions available to all derived classes
// by the Curiously Recursive Template Pattern (CRTP) mechanism
/*
template<class T>
struct i2c_static_base
{
	static bool identifyDevice(uint8_t addr) {
		if ()
		auto it = T::getGlobalDeviceList().begin();
		bool found { false };
		while ( !found && it != T::getGlobalDeviceList().end() ) {
			if ( (*it)->getAddress() == addr) { 
				found = true;
				break;
			}
			it++;
		}
		if ( found ) {
			T dummyDevice( 0x00 );
			if ( (*it)->getTitle() == dummyDevice.getTitle() ) return true;
			return false;
		}
		T device(addr);
		return device.identify();
	}
};
*/

class i2c_device {
public:
	//friend struct i2c_static_base;
	
	enum class Flags : std::uint8_t {
        None = 0,
        Normal = 0x01,
        Force = 0x02,
        Unreachable = 0x04,
        Failed = 0x08,
        Locked = 0x10
    };

    explicit i2c_device(i2c_bus& bus, std::uint8_t address);
	i2c_device(i2c_bus& bus);
	//i2c_device(const i2c_bus& bus);
	
	void set_address( std::uint8_t address );
	[[nodiscard]] auto address() const -> std::uint8_t { return m_address; }
	
    virtual ~i2c_device();

    [[nodiscard]] auto is_open() const -> bool;
    void close();

    void read_capabilities();
    [[nodiscard]] virtual auto present() -> bool;
	[[nodiscard]] virtual auto identify() -> bool;

    [[nodiscard]] auto io_errors() const -> std::size_t;

    [[nodiscard]] auto rx_bytes() const -> std::size_t;
    [[nodiscard]] auto tx_bytes() const -> std::size_t;

    [[nodiscard]] auto flag_set(Flags flag) const -> bool;

    void lock(bool locked = true);
    [[nodiscard]] auto locked() const -> bool;

    [[nodiscard]] auto last_interval() const -> double;

    void set_title(std::string title);
    [[nodiscard]] auto title() const -> std::string;


    [[nodiscard]] auto read(std::uint8_t* buffer, std::size_t bytes = 1) -> int;

    [[nodiscard]] auto read(std::uint8_t reg, std::uint8_t bit_mask) -> std::uint16_t;

    [[nodiscard]] auto read(std::uint8_t reg, std::uint8_t* buffer, std::size_t bytes = 1) -> int;

	[[nodiscard]] auto read(std::uint8_t reg, std::uint16_t* buffer, std::size_t n_words = 1) -> int;

    [[nodiscard]] auto write(std::uint8_t* buffer, std::size_t bytes = 1) -> int;

    [[nodiscard]] auto write(std::uint8_t reg, std::uint8_t bit_mask, std::uint8_t value) -> bool;

    [[nodiscard]] auto write(std::uint8_t reg, std::uint8_t* buffer, std::size_t bytes = 1) -> int;

    [[nodiscard]] auto write(std::uint8_t reg, std::uint16_t* buffer, std::size_t length = 1) -> int;

protected:
    void set_flag(Flags flag);
    void unset_flag(Flags flag);

    void start_timer();
    void stop_timer();

private:
    i2c_bus& m_bus;
	std::uint8_t m_address { 0xff };

    int m_handle {};
    bool m_locked { false };

    std::size_t m_rx_bytes {};
    std::size_t m_tx_bytes {};
/*
    std::size_t& m_rx_counter;
    std::size_t& m_tx_counter;
*/
    std::size_t m_io_errors {};
    double m_last_interval; // the last time measurement's result is stored here

    std::string m_title { "I2C device" };
    std::uint8_t m_flags {};

    std::chrono::system_clock::time_point m_start {};
};

}

#endif // MUONPI_I2CDEVICE_H
