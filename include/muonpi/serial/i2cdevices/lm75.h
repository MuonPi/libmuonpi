#ifndef _LM75_H_
#define _LM75_H_
#include "muonpi/serial/i2cdevice.h"
#include "muonpi/serial/i2cbus.h"


namespace muonpi::serial::devices {

class LM75 : public i2c_device {
public:
	explicit LM75(i2c_bus& bus, std::uint8_t address);

	virtual ~LM75();
    float get_temperature();

    [[nodiscard]] auto identify() -> bool override;
protected:
    int16_t readRaw();

	enum REG : uint8_t {
		TEMP = 0x00,
		CONF = 0x01,
		THYST = 0x02,
		TOS = 0x03
	};
};

} // namespace muonpi::serial::devices

#endif // !_LM75_H_
