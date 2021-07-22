cmake_minimum_required(VERSION 3.10)
project(example-mqtt LANGUAGES CXX C)

set(PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(PROJECT_HEADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PROJECT_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../output/examples")


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wshadow -Wpedantic -Werror -O3)

find_package(
  Boost 1.71
  COMPONENTS system program_options
  REQUIRED)

add_executable(example-mqtt src/main.cpp)

target_link_libraries(example-mqtt
    pthread
    muonpi-core
    muonpi-http
    muonpi-link
    dl
    )
