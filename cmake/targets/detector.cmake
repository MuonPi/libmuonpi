option(LIBMUONPI_BUILD_DETECTOR "Build the detector code" ON )

set(DETECTOR_SOURCE_FILES
    "${PROJECT_SRC_DIR}/gpio_handler.cpp"
    "${PROJECT_SRC_DIR}/serial/spidevice.cpp"
    )

set(DETECTOR_HEADER_FILES
    "${PROJECT_HEADER_DIR}/muonpi/gpio_handler.h"
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
