#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "muonpi/global.h"

#include <boost/stacktrace.hpp>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

namespace muonpi::error {

void inline terminate_handler()
{
    try {
        std::cerr << boost::stacktrace::stacktrace();
    } catch (...) {
    }
    std::abort();
}

}

namespace boost {
void assertion_failed_msg(char const* expr, char const* msg, char const* function, char const* /*file*/, long /*line*/);

void assertion_failed(char const* expr, char const* function, char const* file, long line);
} // namespace boost

#endif // EXCEPTIONS_H
