option(LIBMUONPI_BUILD_LINK "Build the link code" ON )

if (LIBMUONPI_BUILD_LINK) # libraries specific to the link library
    set(LINK_SOURCE_FILES
        "${PROJECT_SRC_DIR}/link/mqtt.cpp"
        "${PROJECT_SRC_DIR}/link/influx.cpp"
        )

    set(LINK_HEADER_FILES
        "${PROJECT_HEADER_DIR}/muonpi/link/mqtt.h"
        "${PROJECT_HEADER_DIR}/muonpi/link/influx.h"
        )


    find_library(MOSQUITTO mosquitto REQUIRED)


    add_library(muonpi-link SHARED ${LINK_SOURCE_FILES} ${LINK_HEADER_FILES})
    add_dependencies(muonpi-link muonpi-core)
    target_link_libraries(muonpi-link ${PROJECT_INCLUDE_LIBS} muonpi-http mosquitto)



    setup_packaging(
        COMPONENT "link"
        HEADERS "${LINK_HEADER_FILES}"
        DESCRIPTION "Libraries for MuonPi
     Link package"
        DESCRIPTIONDEV "Libraries for MuonPi
     Link dev package")
endif ()
