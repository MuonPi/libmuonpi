#ifndef MUONPI_SERIAL_I2CDEVICES_H
#define MUONPI_SERIAL_I2CDEVICES_H

#include "muonpi/serial/i2cdevice.h"
#include "muonpi/serial/i2cdevices/ads1x_adc.h"
#include "muonpi/serial/i2cdevices/eeproms.h"
#include "muonpi/serial/i2cdevices/generic_eeprom.h"
#include "muonpi/serial/i2cdevices/hmc5883.h"
#include "muonpi/serial/i2cdevices/lm75.h"
#include "muonpi/serial/i2cdevices/mcp4728.h"
#include "muonpi/serial/i2cdevices/mic184.h"
#include "muonpi/serial/i2cdevices/pca9536.h"
#include "muonpi/serial/i2cdevices/pca9554.h"

namespace muonpi::serial::devices {

typedef ADS1X_ADC<4, 12>        ADS1015;
typedef ADS1X_ADC<1, 12>        ADS1014;
typedef ADS1X_ADC<1, 12, false> ADS1013;
typedef ADS1X_ADC<4, 16>        ADS1115;
typedef ADS1X_ADC<1, 16>        ADS1114;
typedef ADS1X_ADC<1, 16, false> ADS1113;

} // namespace muonpi::serial::devices

#endif // MUONPI_SERIAL_I2CDEVICES_H
