#include <muonpi/link/mqtt.h>
#include <muonpi/log.h>

#include <iostream>

auto main() -> int
{
    muonpi::log::system::setup(muonpi::log::Level::Info);

    muonpi::link::mqtt::configuration config {};

    config.host = "muonpi.org";
    config.login.username = "username";
    config.login.password = "password";

    muonpi::link::mqtt mqtt{config, "mqtt-example"};

    if (!mqtt.wait_for(muonpi::link::mqtt::Status::Connected)) {
        muonpi::log::critical(1)<<"Could not connect to mqtt.";
    }

    auto& subscriber = mqtt.subscribe("muonpi/example/#");
    auto& publisher = mqtt.publish("muonpi/example");

    subscriber.emplace_callback([](const muonpi::link::mqtt::message_t& message){
        muonpi::log::info()<<"Received message: "<<message.topic<<" -> "<<message.content;
    });

    std::this_thread::sleep_for(std::chrono::seconds{1});
    if (!publisher.publish("Hello! here is a quick test!")) {
        muonpi::log::warning()<<"Could not publish messge.";
    }

    std::this_thread::sleep_for(std::chrono::seconds{1});
    if (!publisher.publish("test", "Hello! here is a second quick test!")) {
        muonpi::log::warning()<<"Could not publish messge.";
    }

    std::this_thread::sleep_for(std::chrono::seconds{1});
    mqtt.stop();
    return mqtt.wait();
}
