#ifndef LIBMUONPI_VERSION_H
#define LIBMUONPI_VERSION_H

#include <chrono>
#include <string>
#include <memory>

#ifdef _MSC_VER
    //  Microsoft
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    //  GCC
    #define EXPORT __attribute__((visibility("default")))
    #define IMPORT
#else
    //  do nothing and hope for the best?
    #define EXPORT
    #define IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif

#define LIBMUONPI_COMPILING

#ifdef LIBMUONPI_COMPILING
#   define LIBMUONPI_PUBLIC EXPORT
#else
#   define LIBMUONPI_PUBLIC IMPORT
#endif

#define BOOST_ENABLE_ASSERT_DEBUG_HANDLER

namespace muonpi::Version {
constexpr int major { 0 };
constexpr int minor { 1 };
constexpr int patch { 0 };
constexpr const char* additional { "" };

[[nodiscard]] auto string() -> std::string;

}

#endif // LIBMUONPI_VERSION_H
