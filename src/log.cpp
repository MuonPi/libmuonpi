#include "muonpi/log.h"

namespace muonpi::log {

auto debug() -> logger<Level::Debug>
{
    return {};
}

auto info() -> logger<Level::Info>
{
    return {};
}

auto notice() -> logger<Level::Notice>
{
    return {};
}

auto warning() -> logger<Level::Warning>
{
    return {};
}

auto error() -> logger<Level::Error>
{
    return {};
}

auto critical(int exit_code) -> logger<Level::Critical>
{
    return { exit_code };
}

auto alert(int exit_code) -> logger<Level::Alert>
{
    return { exit_code };
}

auto emergency(int exit_code) -> logger<Level::Emergency>
{
    return { exit_code };
}

} // namespace muonpi::log
