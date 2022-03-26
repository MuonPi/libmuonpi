cmake_minimum_required(VERSION 3.10)
project(libmuonpi LANGUAGES CXX C)

string(TIMESTAMP PROJECT_DATE_STRING "%b %d, %Y")

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wshadow -Wpedantic -Werror -O3)


option(LIBMUONPI_BUILD_SANITIZERS "Build sanitizers into the libraries" OFF)
option(LIBMUONPI_BUILD_EXAMPLES "Build examples for how to use the library" ON)
option(LIBMUONPI_TESTS "Build unit tests" OFF)
option(LIBMUONPI_HARDWARE_TESTS "Build unit tests which depend on specific hardware" OFF)
option(LIBMUONPI_BUILD_CLANG_TIDY "Run clang-tidy" OFF)
option(LIBMUONPI_FORMAT_ONLY "Don't actually build the project. Only create the format target" OFF)

if (LIBMUONPI_BUILD_CLANG_TIDY)
set(CMAKE_CXX_CLANG_TIDY clang-tidy  -checks=-*,bugprone-*,clang-*,cppcoreguidelines-*,hicpp-*,llvm-*,misc-*,modernize-*,performance-*,portability-*,readability-*,-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers)
endif()



set(LIBMUONPI_COMPILING ON)

set(PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(PROJECT_TEST_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
set(PROJECT_HEADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PROJECT_DETAIL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/detail")
set(PROJECT_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output/lib")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output/packages/")
include_directories(${PROJECT_HEADER_DIR} ${PROJECT_SRC_DIR})


if (CMAKE_BUILD_TYPE STREQUAL Release)
    set(LIBMUONPI_IS_RELEASE ON)
else()
    set(LIBMUONPI_IS_RELEASE OFF)
endif()


if (NOT LIBMUONPI_FORMAT_ONLY)
set(USE_BOOST_COMPONENTS system program_options)
if (LIBMUONPI_TESTS)
set(USE_BOOST_COMPONENTS ${USE_BOOST_COMPONENTS} unit_test_framework)
endif ()

find_library(DL dl REQUIRED)

find_package(
  Boost 1.67
  COMPONENTS ${USE_BOOST_COMPONENTS}
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
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # rationale for disabling this warning: https://stackoverflow.com/questions/48149323/what-does-the-gcc-warning-project-parameter-passing-for-x-changed-in-gcc-7-1-m/48149400#48149400
        add_compile_options(-Wno-psabi)
    endif ()

endif()
# --- necessary for compatability with older compilers

configure_file("${PROJECT_CONFIG_DIR}/config.h" "${PROJECT_HEADER_DIR}/muonpi/global.h")
endif()


include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/macros.cmake")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/targets/core.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/targets/http.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/targets/link.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/targets/detector.cmake")

if (NOT LIBMUONPI_FORMAT_ONLY)
if(${LIBMUONPI_BUILD_SANITIZERS})
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif()


if(${LIBMUONPI_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif()

if(LIBMUONPI_TESTS)
    add_subdirectory(tests)
endif()
endif ()

set(PROJECT_SOURCE_FILES
    ${CORE_SOURCE_FILES}
    ${LINK_SOURCE_FILES}
    ${HTTP_SOURCE_FILES}
    ${DETECTOR_SOURCE_FILES}
    )

set(PROJECT_HEADER_FILES
    ${CORE_HEADER_FILES}
    ${LINK_HEADER_FILES}
    ${HTTP_HEADER_FILES}
    ${DETECTOR_HEADER_FILES}
    )

add_custom_target(clangformat WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMAND clang-format-11 -style=file -i ${PROJECT_SOURCE_FILES} ${PROJECT_HEADER_FILES})

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/packaging.cmake")
