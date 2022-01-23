#ifndef _MIC184_H_
#define _MIC184_H_
#include "muonpi/serial/i2cdevice.h"
#include "muonpi/serial/i2cbus.h"


namespace muonpi::serial::devices {

class MIC184 : public i2c_device {
public:
    explicit MIC184(i2c_bus& bus, std::uint8_t address);
	virtual ~MIC184();

    float get_temperature();
        
	bool identify() override;
	bool is_external() const { return fExternal; }
	bool set_external( bool enable_external = true );
private:
	int16_t readRaw();

	enum REG : uint8_t {
		TEMP = 0x00,
		CONF = 0x01,
		THYST = 0x02,
		TOS = 0x03
	};

	bool fExternal { false };
};

} // namespace muonpi::serial::devices
#endif // !_MIC184_H_
