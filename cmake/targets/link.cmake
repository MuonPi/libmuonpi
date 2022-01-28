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

    if (LIBMUONPI_TESTS)
        set(LINK_TEST_SOURCE_FILES
            "${PROJECT_TEST_SRC_DIR}/link/main.cpp"
            )
        add_executable(muonpi-link-test ${LINK_TEST_SOURCE_FILES})
        target_link_libraries(muonpi-link-test ${PROJECT_INCLUDE_LIBS} muonpi-link muonpi-http mosquitto)
        add_test(muonpi-link-test muonpi-link-test)
    endif ()


    setup_packaging(
        COMPONENT "link"
        HEADERS "${LINK_HEADER_FILES}"
        DESCRIPTION "Libraries for MuonPi
     Link package"
        DESCRIPTIONDEV "Libraries for MuonPi
     Link dev package")
endif ()
