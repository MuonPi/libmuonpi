#ifndef MUONPI_EXCEPTIONS_H
#define MUONPI_EXCEPTIONS_H

#include "muonpi/global.h"

#include <boost/stacktrace.hpp>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

namespace muonpi::error {

void inline terminate_handler() {
    try {
        std::cerr << boost::stacktrace::stacktrace();
    } catch (...) {}
    std::abort();
}

} // namespace muonpi::error

#endif // MUONPI_EXCEPTIONS_H
