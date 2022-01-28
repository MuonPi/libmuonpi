option(LIBMUONPI_BUILD_DETECTOR "Build the detector code" ON )

if (LIBMUONPI_BUILD_DETECTOR) # libraries specific to the Detector library
    set(DETECTOR_SOURCE_FILES
        "${PROJECT_SRC_DIR}/gpio_handler.cpp"
        "${PROJECT_SRC_DIR}/serial/i2cdevice.cpp"
        "${PROJECT_SRC_DIR}/serial/i2cbus.cpp"
        "${PROJECT_SRC_DIR}/serial/i2cdevices/lm75.cpp"
        "${PROJECT_SRC_DIR}/serial/i2cdevices/mic184.cpp"
        "${PROJECT_SRC_DIR}/serial/i2cdevices/ads1115.cpp"
        "${PROJECT_SRC_DIR}/serial/i2cdevices/mcp4728.cpp"
        "${PROJECT_SRC_DIR}/serial/i2cdevices/eeprom24aa02.cpp"
        "${PROJECT_SRC_DIR}/serial/i2cdevices/pca9536.cpp"
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
        "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevices/eeprom24aa02.h"
        "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevices/pca9536.h"
        )

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
