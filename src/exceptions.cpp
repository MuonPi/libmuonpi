#include "muonpi/exceptions.h"

namespace boost {

void assertion_failed_msg(char const* expr,
    char const* msg,
    char const* function,
    char const* /*file*/,
    long /*line*/)
{
    std::cerr << "Expression '" << expr << "' is false in function '" << function
              << "': " << (msg != nullptr ? msg : "<...>") << ".\n"
              << "Backtrace:\n"
              << boost::stacktrace::stacktrace() << '\n';
    std::abort();
}

void assertion_failed(char const* expr, char const* function, char const* file, long line)
{
    boost::assertion_failed_msg(expr, nullptr, function, file, line);
}

} // namespace boost
