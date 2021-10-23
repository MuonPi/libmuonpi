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
    #define HIDDEN  __attribute__((visibility("hidden")))
#else
    //  do nothing and hope for the best?
    #define EXPORT
    #define IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif

#cmakedefine LIBMUONPI_COMPILING

#ifdef LIBMUONPI_COMPILING
#   define LIBMUONPI_PUBLIC EXPORT
#else
#   define LIBMUONPI_PUBLIC IMPORT
#endif

#define BOOST_ENABLE_ASSERT_DEBUG_HANDLER

namespace muonpi::Version::libmuonpi {
constexpr int major { @PROJECT_VERSION_MAJOR@ };
constexpr int minor { @PROJECT_VERSION_MINOR@ };
constexpr int patch { @PROJECT_VERSION_PATCH@ };
constexpr const char* additional { "@PROJECT_VERSION_ADDITIONAL@" };

[[nodiscard]] auto string() -> std::string;

}

#endif // LIBMUONPI_VERSION_H
