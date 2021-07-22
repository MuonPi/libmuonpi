#include <muonpi/link/influx.h>
#include <muonpi/log.h>

#include <iostream>

auto main() -> int
{
    muonpi::log::system::setup(muonpi::log::Level::Info);

    muonpi::link::influx::configuration config {};
    config.database = "Test-db";
    config.host = "127.0.0.1";
    config.login.username = "username";
    config.login.password = "password";

    muonpi::link::influx database{config};

    auto entry = database.measurement("test");

    using tag = muonpi::link::influx::tag;
    using field = muonpi::link::influx::field;

    entry
            <<tag{"tag-name", "value"}
            <<tag{"tag2", "value2"}
            <<field{"field1", 5}
               ;

    const std::int_fast64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if (entry.commit(timestamp)) {
        muonpi::log::info()<<"wrote measurement.";
    } else {
        muonpi::log::warning()<<"Couldn't write to database.";
    }
}
