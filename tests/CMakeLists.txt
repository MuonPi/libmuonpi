cmake_minimum_required(VERSION 3.10)
project(libmuonpi LANGUAGES CXX C)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wshadow -Wpedantic -Werror -O3)

set(PROJECT_TEST_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../tests")
set(PROJECT_HEADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")

include_directories(${PROJECT_HEADER_DIR})


find_package(
  Boost 1.67
  COMPONENTS system program_options unit_test_framework
  REQUIRED)

set(PROJECT_INCLUDE_LIBS
    pthread
    dl
    ${Boost_LIBRARIES}
    )

# +++ necessary for compatability with older compilers
find_library(STD_CPP_FS stdc++fs /usr/lib/gcc/x86_64-linux-gnu/8/)

if(STD_CPP_FS)
    set(PROJECT_INCLUDE_LIBS ${PROJECT_INCLUDE_LIBS} stdc++fs)
endif()
# --- necessary for compatability with older compilers

set(CORE_TEST_SOURCE_FILES
    "${PROJECT_TEST_SRC_DIR}/core/main.cpp"
    "${PROJECT_TEST_SRC_DIR}/core/utilities.cpp"
    "${PROJECT_TEST_SRC_DIR}/core/threadrunner.cpp"
    "${PROJECT_TEST_SRC_DIR}/core/dataseries.cpp"
    "${PROJECT_TEST_SRC_DIR}/core/ratemeasurement.cpp"
    "${PROJECT_TEST_SRC_DIR}/core/gnss.cpp"
    )
add_executable(core-test ${CORE_TEST_SOURCE_FILES})
target_link_libraries(core-test ${PROJECT_INCLUDE_LIBS} muonpi-core)
add_test(core-test core-test)


if (LIBMUONPI_BUILD_DETECTOR)
set(DETECTOR_TEST_SOURCE_FILES
    "${PROJECT_TEST_SRC_DIR}/detector/main.cpp"
    "${PROJECT_TEST_SRC_DIR}/detector/gpio.cpp"
    )
add_executable(detector-test ${DETECTOR_TEST_SOURCE_FILES})
target_link_libraries(detector-test ${PROJECT_INCLUDE_LIBS} muonpi-detector muonpi-core gpiod atomic)
add_test(detector-test detector-test)
endif ()

if (LIBMUONPI_HARDWARE_TESTS)
    set(HARDWARE_TEST_SOURCE_FILES
    "${PROJECT_TEST_SRC_DIR}/hardware/main.cpp"
    "${PROJECT_TEST_SRC_DIR}/hardware/gpio.cpp"
    "${PROJECT_TEST_SRC_DIR}/hardware/i2c.cpp"
    )
add_executable(hardware-test ${HARDWARE_TEST_SOURCE_FILES})
target_link_libraries(hardware-test ${PROJECT_INCLUDE_LIBS} muonpi-detector muonpi-core gpiod atomic)
add_test(hardware-test hardware-test)
endif ()

if (LIBMUONPI_BUILD_HTTP)
set(HTTP_TEST_SOURCE_FILES
    "${PROJECT_TEST_SRC_DIR}/http/main.cpp"
    )
add_executable(http-test ${HTTP_TEST_SOURCE_FILES})
target_link_libraries(http-test ${PROJECT_INCLUDE_LIBS} muonpi-http muonpi-core ssl crypto)
add_test(http-test http-test)
endif ()

if (LIBMUONPI_BUILD_LINK)
set(LINK_TEST_SOURCE_FILES
    "${PROJECT_TEST_SRC_DIR}/link/main.cpp"
    )
add_executable(link-test ${LINK_TEST_SOURCE_FILES})
target_link_libraries(link-test ${PROJECT_INCLUDE_LIBS} muonpi-link muonpi-http mosquitto)
add_test(link-test link-test)
endif ()

enable_testing()
