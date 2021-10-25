cmake_minimum_required(VERSION 3.10)
project(libmuonpi LANGUAGES CXX C)

string(TIMESTAMP PROJECT_DATE_STRING "%b %d, %Y")


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wshadow -Wpedantic -Werror -O3)


option(LIBMUONPI_BUILD_SANITIZERS "Build sanitizers into the libraries" OFF)
option(LIBMUONPI_BUILD_EXAMPLES "Build examples for how to use the library" OFF)

set(LIBMUONPI_COMPILING ON)

set(PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(PROJECT_HEADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PROJECT_DETAIL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/detail")
set(PROJECT_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output/lib")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output/packages/")
include_directories(${PROJECT_HEADER_DIR} ${PROJECT_SRC_DIR})


if (CMAKE_BUILD_TYPE STREQUAL Release)
    set(LIBMUONPI_IS_RELEASE ON)
else()
    set(LIBMUONPI_IS_RELEASE OFF)
endif()




find_package(
  Boost 1.67
  COMPONENTS system program_options
  REQUIRED)
set(PROJECT_INCLUDE_LIBS
    pthread
    ${Boost_LIBRARIES}
    )

# +++ necessary for compatability with older compilers
find_library(STD_CPP_FS stdc++fs /usr/lib/gcc/x86_64-linux-gnu/8/)

if(STD_CPP_FS)
    set(PROJECT_INCLUDE_LIBS ${PROJECT_INCLUDE_LIBS} stdc++fs)
endif()
# --- necessary for compatability with older compilers

configure_file("${PROJECT_CONFIG_DIR}/config.h" "${PROJECT_HEADER_DIR}/muonpi/global.h")



include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/macros.cmake")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/targets/core.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/targets/http.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/targets/link.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/targets/detector.cmake")

if(${LIBMUONPI_BUILD_SANITIZERS})
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif()


if(${LIBMUONPI_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif()


set(PROJECT_SOURCE_FILES
    ${CORE_SOURCE_FILES}
    ${LINK_SOURCE_FILES}
    ${HTTP_SOURCE_FILES}
    )

set(PROJECT_HEADER_FILES
    ${CORE_HEADER_FILES}
    ${LINK_HEADER_FILES}
    ${HTTP_HEADER_FILES}
    )

add_custom_target(clangformat COMMAND clang-format -style=WebKit -i ${PROJECT_SOURCE_FILES} ${PROJECT_HEADER_FILES})

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/packaging.cmake")
