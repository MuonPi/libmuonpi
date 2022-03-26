option(LIBMUONPI_BUILD_DETECTOR "Build the detector code" ON )

set(DETECTOR_SOURCE_FILES
    "${PROJECT_SRC_DIR}/gpio_handler.cpp"
    "${PROJECT_SRC_DIR}/serial/i2cdevice.cpp"
    "${PROJECT_SRC_DIR}/serial/i2cbus.cpp"
    "${PROJECT_SRC_DIR}/serial/i2cdevices/lm75.cpp"
    "${PROJECT_SRC_DIR}/serial/i2cdevices/mic184.cpp"
    "${PROJECT_SRC_DIR}/serial/i2cdevices/ads1115.cpp"
    "${PROJECT_SRC_DIR}/serial/i2cdevices/mcp4728.cpp"
    "${PROJECT_SRC_DIR}/serial/i2cdevices/hmc5883.cpp"
    "${PROJECT_SRC_DIR}/serial/spidevice.cpp"
    )

set(DETECTOR_HEADER_FILES
    "${PROJECT_HEADER_DIR}/muonpi/gpio_handler.h"
    "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevice.h"
    "${PROJECT_HEADER_DIR}/muonpi/serial/i2cbus.h"
    "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevices.h"
    "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevices/lm75.h"
    "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevices/mic184.h"
    "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevices/ads1115.h"
    "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevices/mcp4728.h"
    "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevices/generic_eeprom.h"
    "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevices/eeproms.h"
    "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevices/io_extender.h"
    "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevices/pca9536.h"
    "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevices/pca9554.h"
    "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevices/hmc5883.h"
    "${PROJECT_HEADER_DIR}/muonpi/serial/spidevice.h"
    )
if (NOT LIBMUONPI_FORMAT_ONLY)
if (LIBMUONPI_BUILD_DETECTOR) # libraries specific to the Detector library
    find_library(LIBGPIOD gpiod REQUIRED)


    add_library(muonpi-detector SHARED ${DETECTOR_SOURCE_FILES} ${DETECTOR_HEADER_FILES})
    add_dependencies(muonpi-detector muonpi-core)
    target_link_libraries(muonpi-detector ${PROJECT_INCLUDE_LIBS} muonpi-core gpiod atomic)

    setup_packaging(
        COMPONENT "detector"
        HEADERS "${DETECTOR_HEADER_FILES}"
        DESCRIPTION "Libraries for MuonPi
     Link package"
        DESCRIPTIONDEV "Libraries for MuonPi
     Link dev package")
endif ()
endif ()
