cmake_minimum_required(VERSION 3.10)
project(example-http-request LANGUAGES CXX C)

set(PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(PROJECT_HEADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PROJECT_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../output/examples")


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wshadow -Wpedantic -Werror -O3)

find_package(
  Boost 1.67
  COMPONENTS system program_options
  REQUIRED)

add_executable(example-http-request src/main.cpp)

target_link_libraries(example-http-request
    pthread
    ${Boost_LIBRARIES}
    ssl
    crypto
    muonpi-core
    muonpi-http
    dl
    )
