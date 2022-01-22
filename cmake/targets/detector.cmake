option(LIBMUONPI_BUILD_DETECTOR "Build the detector code" ON )

if (LIBMUONPI_BUILD_DETECTOR) # libraries specific to the Detector library
    set(DETECTOR_SOURCE_FILES
        "${PROJECT_SRC_DIR}/serial/i2cdevice.cpp"
        "${PROJECT_SRC_DIR}/serial/i2cbus.cpp"
        "${PROJECT_SRC_DIR}/serial/i2cdevices/lm75.cpp"
        )

    set(DETECTOR_HEADER_FILES
        "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevice.h"
        "${PROJECT_HEADER_DIR}/muonpi/serial/i2cbus.h"
        "${PROJECT_HEADER_DIR}/muonpi/serial/i2cdevices//lm75.h"
        )

    find_library(LIBGPIOD gpiod REQUIRED)


    add_library(muonpi-detector SHARED ${DETECTOR_SOURCE_FILES} ${DETECTOR_HEADER_FILES})
    add_dependencies(muonpi-detector muonpi-core)
    target_link_libraries(muonpi-detector ${PROJECT_INCLUDE_LIBS} muonpi-core gpiod)

    setup_packaging(
        COMPONENT "detector"
        HEADERS "${DETECTOR_HEADER_FILES}"
        DESCRIPTION "Libraries for MuonPi
     Link package"
        DESCRIPTIONDEV "Libraries for MuonPi
     Link dev package")
endif ()
