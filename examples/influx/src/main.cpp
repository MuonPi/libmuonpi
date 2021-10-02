#include <muonpi/link/influx.h>
#include <muonpi/log.h>

#include <iostream>

using tag = muonpi::link::influx::tag;
template <typename T>
using field = muonpi::link::influx::field<T>;

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


    entry
            <<tag{"tag-name", "value"}
            <<tag{"tag2", "value2"}
            <<field<int>{"field1", 5}
            <<field<double>{"field2", 5.53}
               ;

    const std::int_fast64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if (entry.commit(timestamp)) {
        muonpi::log::info()<<"wrote measurement.";
    } else {
        muonpi::log::warning()<<"Couldn't write to database.";
    }
}
