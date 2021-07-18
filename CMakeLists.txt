cmake_minimum_required(VERSION 3.10)
project(libmuonpi LANGUAGES CXX C)

string(TIMESTAMP PROJECT_DATE_STRING "%b %d, %Y")


option(LIBMUONPI_BUILD_MQTT "Build the mqtt code" ON )
option(LIBMUONPI_BUILD_REST "Build the REST service code" ON)
option(LIBMUONPI_BUILD_DETECTOR "Build the detector code" ON)
option(LIBMUONPI_BUILD_SANITIZERS "Build sanitizers into the libraries" OFF)

set(LIBMUONPI_COMPILING ON)

set(PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(PROJECT_HEADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PROJECT_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output/lib")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output/packages/")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/files.cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wshadow -Wpedantic -Werror -O3)

if(${LIBMUONPI_BUILD_SANITIZERS})
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif()



find_package(
  Boost 1.71
  COMPONENTS system
  REQUIRED)

if (LIBMUONPI_BUILD_MQTT) # libraries specific to the MQTT library
    find_library(MOSQUITTO mosquitto REQUIRED)
endif ()

if (LIBMUONPI_BUILD_REST) # libraries specific to the REST libraries
    find_library(DL dl REQUIRED)
endif ()

if (LIBMUONPI_BUILD_DETECTOR) # libraries specific to the REST libraries
endif ()

set(PROJECT_INCLUDE_LIBS
    pthread
    boost_system
    )

# +++ necessary for compatability with older compilers
find_library(STD_CPP_FS stdc++fs /usr/lib/gcc/x86_64-linux-gnu/8/)

if(STD_CPP_FS)
    set(PROJECT_INCLUDE_LIBS ${PROJECT_INCLUDE_LIBS} stdc++fs)
endif()
# --- necessary for compatability with older compilers

configure_file("${PROJECT_CONFIG_DIR}/config.h" "${PROJECT_HEADER_DIR}/muonpi/global.h")

add_custom_target(clangformat COMMAND clang-format -style=WebKit -i ${PROJECT_SOURCE_FILES} ${PROJECT_HEADER_FILES})

add_library(muonpi-core SHARED ${CORE_SOURCE_FILES} ${CORE_HEADER_FILES})
target_include_directories(muonpi-core PUBLIC ${PROJECT_HEADER_DIR})
target_link_libraries(muonpi-core ${PROJECT_INCLUDE_LIBS})

if (LIBMUONPI_BUILD_MQTT)
    add_library(muonpi-mqtt SHARED ${MQTT_SOURCE_FILES} ${MQTT_HEADER_FILES})
    add_dependencies(muonpi-mqtt muonpi-core)
    target_include_directories(muonpi-mqtt PUBLIC ${PROJECT_HEADER_DIR})
    target_link_libraries(muonpi-mqtt ${PROJECT_INCLUDE_LIBS} mosquitto)
endif ()

if (LIBMUONPI_BUILD_REST)
    add_library(muonpi-rest SHARED ${REST_SOURCE_FILES} ${REST_HEADER_FILES})
    add_dependencies(muonpi-rest muonpi-core)
    target_include_directories(muonpi-rest PUBLIC ${PROJECT_HEADER_DIR})
    target_link_libraries(muonpi-rest ${PROJECT_INCLUDE_LIBS} muonpi-core ssl crypto dl)
endif ()

if (LIBMUONPI_BUILD_DETECTOR)
    add_library(muonpi-detector SHARED ${DETECTOR_SOURCE_FILES} ${DETECTOR_HEADER_FILES})
    add_dependencies(muonpi-detector muonpi-core)
    target_include_directories(muonpi-detector PUBLIC ${PROJECT_HEADER_DIR})
    target_link_libraries(muonpi-detector ${PROJECT_INCLUDE_LIBS})
endif ()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/packaging.cmake")
