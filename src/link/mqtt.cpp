#include "muonpi/link/mqtt.h"
#include "muonpi/exceptions.h"
#include "muonpi/log.h"

#include <cstring>
#include <functional>
#include <regex>
#include <sstream>
#include <thread>
#include <utility>

namespace muonpi::link {

constexpr std::chrono::duration s_timeout { std::chrono::milliseconds { 10 } };

auto mqtt::wait_for(Status status, std::chrono::milliseconds duration) -> bool
{
    std::chrono::steady_clock::time_point start { std::chrono::steady_clock::now() };
    while (((std::chrono::steady_clock::now() - start) < duration)) {
        if (m_status == status) {
            return true;
        }
        std::this_thread::sleep_for(s_timeout);
    }
    return false;
}

void wrapper_callback_connected(mosquitto* /*mqtt*/, void* object, int result)
{
    static_cast<mqtt*>(object)->callback_connected(result);
}

void wrapper_callback_disconnected(mosquitto* /*mqtt*/, void* object, int result)
{
    static_cast<mqtt*>(object)->callback_disconnected(result);
}

void wrapper_callback_message(mosquitto* /*mqtt*/, void* object, const mosquitto_message* message)
{
    static_cast<mqtt*>(object)->callback_message(message);
}

mqtt::mqtt(configuration config, std::string station_id, const std::string& name)
    : thread_runner { name }
    , m_config { std::move(config) }
    , m_station_id { std::move(station_id) }
    , m_mqtt { init(client_id().c_str()) }
{
    mosquitto_connect_callback_set(m_mqtt, wrapper_callback_connected);
    mosquitto_disconnect_callback_set(m_mqtt, wrapper_callback_disconnected);
    mosquitto_message_callback_set(m_mqtt, wrapper_callback_message);

    start();
}

mqtt::mqtt(std::string station_id, const std::string& name)
    : thread_runner { name }
    , m_station_id { std::move(station_id) }
    , m_mqtt { init(client_id().c_str()) }
{
}

mqtt::~mqtt()
    = default;

auto mqtt::pre_run() -> int
{
    if (m_mqtt == nullptr) {
        return -1;
    }
    mosquitto_loop_start(m_mqtt);
    if (!connect()) {
        return -1;
    }
    if ((m_status != Status::Connected) && (m_status != Status::Connecting)) {
        return -1;
    }
    return 0;
}

auto mqtt::step() -> int
{
    std::mutex mx;
    std::unique_lock<std::mutex> lock { mx };
    m_condition.wait(lock);
    if (!m_quit) {
        if (!connect()) {
            return -1;
        }
    }
    return 0;
}

void mqtt::callback_connected(int result)
{
    if (!m_connect_future.valid()) {
        return;
    }
    switch (result) {
    case MOSQ_ERR_SUCCESS:
        log::info("mqtt") << "Connected.";
        set_status(Status::Connected);
        m_tries = 0;
        for (auto& [topic, sub] : m_subscribers) {
            p_subscribe(topic);
        }
        m_connect_promise.set_value(true);
        return;
    case MOSQ_ERR_PROTOCOL:
        log::warning("mqtt") << "Connection failed: Protocol Error";
        break;
    case MOSQ_ERR_INVAL:
        log::warning("mqtt") << "Connection failed: invalid value provided";
        break;
    case MOSQ_ERR_CONN_REFUSED:
        log::warning("mqtt") << "Connection failed: Connection refused";
        break;
    case MOSQ_ERR_AUTH:
        log::warning("mqtt") << "Connection failed: Authentication";
        break;
    default:
        log::warning("mqtt") << "Connection failed: Other reason: " << result;
        break;
    };
    set_status(Status::Error);
    m_connect_promise.set_value(false);
}

void mqtt::callback_disconnected(int result)
{
    if (result != 0) {
        if ((m_status == Status::Error) || (m_status == Status::Disconnected)) {
            return;
        }
        log::warning("mqtt") << "Disconnected unexpectedly: " << result;
        m_condition.notify_all();
        set_status(Status::Error);
    } else {
        set_status(Status::Disconnected);
    }
}

void mqtt::callback_message(const mosquitto_message* message)
{
    std::string message_topic { message->topic };
    std::vector<std::string> messages {};
    {
        std::istringstream stream { std::string { static_cast<char*>(message->payload) } };
        std::string line {};
        while (std::getline(stream, line, '\n')) {
            if (!line.empty()) {
                messages.emplace_back(line);
            }
        }
    }
    for (auto& [topic, sub] : m_subscribers) {
        bool result {};
        mosquitto_topic_matches_sub2(topic.c_str(), topic.length(), message_topic.c_str(), message_topic.length(), &result);
        if (!result) {
            continue;
        }
        for (const auto& content : messages) {
            sub->push_message({ message_topic, content });
        }
    }
}

auto mqtt::post_run() -> int
{
    m_subscribers.clear();
    m_publishers.clear();

    if (!disconnect()) {
        return -1;
    }
    if (m_mqtt != nullptr) {
        mosquitto_loop_stop(m_mqtt, true);
        mosquitto_destroy(m_mqtt);
        m_mqtt = nullptr;
        mosquitto_lib_cleanup();
    }
    return 0;
}

void mqtt::on_stop()
{
    m_connect_condition.notify_all();
}

auto mqtt::publish(const std::string& topic, const std::string& content) -> bool
{
    if (!check_connection()) {
        return false;
    }
    auto result { mosquitto_publish(m_mqtt, nullptr, topic.c_str(), static_cast<int>(content.size()), static_cast<const void*>(content.c_str()), 1, false) };
    if (result == MOSQ_ERR_SUCCESS) {
        return true;
    }
    log::warning("mqtt") << "Could not publish message: " << result;
    return false;
}

void mqtt::unsubscribe(const std::string& topic)
{
    if (!check_connection()) {
        return;
    }
    log::info("mqtt") << "Unsubscribing from " << topic;
    mosquitto_unsubscribe(m_mqtt, nullptr, topic.c_str());
}

auto mqtt::publish(const std::string& topic) -> publisher&
{
    if (!check_connection()) {
        log::error("mqtt") << "Could not register publisher: Not connected.";
        throw error::mqtt_could_not_publish(topic, "Not connected");
    }
    if (m_publishers.find(topic) != m_publishers.end()) {
        return { *m_publishers[topic] };
    }
    m_publishers[topic] = std::make_unique<publisher>(this, topic);
    log::info("mqtt") << "Starting to publish on topic " << topic;
    return { *m_publishers[topic] };
}

auto mqtt::check_connection() -> bool
{
    if (m_status != Status::Connected) {
        if (m_status == Status::Connecting) {
            if (!wait_for(Status::Connected)) {
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}

auto mqtt::p_subscribe(const std::string& topic) -> bool
{
    auto result { mosquitto_subscribe(m_mqtt, nullptr, topic.c_str(), 1) };
    if (result != MOSQ_ERR_SUCCESS) {
        switch (result) {
        case MOSQ_ERR_INVAL:
            log::error("mqtt") << "Could not subscribe to topic '" << topic << "': invalid parameters";
            break;
        case MOSQ_ERR_NOMEM:
            log::error("mqtt") << "Could not subscribe to topic '" << topic << "': memory exceeded";
            break;
        case MOSQ_ERR_NO_CONN:
            log::error("mqtt") << "Could not subscribe to topic '" << topic << "': Not connected";
            break;
        case MOSQ_ERR_MALFORMED_UTF8:
            log::error("mqtt") << "Could not subscribe to topic '" << topic << "': malformed utf8";
            break;
        default:
            log::error("mqtt") << "Could not subscribe to topic '" << topic << "': other reason";
            break;
        }
        return false;
    }
    log::info("mqtt") << "Subscribed to topic '" << topic << "'.";
    return true;
}

auto mqtt::subscribe(const std::string& topic) -> subscriber&
{
    if (!check_connection()) {
        log::error("mqtt") << "Could not register subscriber: Not connected.";
        throw error::mqtt_could_not_subscribe(topic, "Not connected");
    }

    if (m_subscribers.find(topic) != m_subscribers.end()) {
        log::info("mqtt") << "Could not register subscriber: Already subscribed.";
        return { *m_subscribers[topic] };
    }

    if (!p_subscribe(topic)) {
        log::error("mqtt") << "Could not register subscriber.";
        throw error::mqtt_could_not_subscribe(topic, "undisclosed error");
    }
    m_subscribers[topic] = std::make_unique<subscriber>(this, topic);
    return { *m_subscribers[topic] };
}

auto mqtt::connect() -> bool
{
    std::mutex mx;
    std::unique_lock<std::mutex> lock { mx };

    if (m_connect_condition.wait_for(lock, std::chrono::seconds { m_tries }) == std::cv_status::no_timeout) {
        return false;
    }

    if (check_connection()) {
        log::notice("mqtt") << "Already connected.";
        return true;
    }

    log::info("mqtt") << "Trying to connect.";
    m_tries++;
    set_status(Status::Connecting);

    if (m_tries > m_config.max_retries) {
        set_status(Status::Error);
        log::error("mqtt") << "Giving up trying to connect.";
        return false;
    }
    if (mosquitto_username_pw_set(m_mqtt, m_config.login.username.c_str(), m_config.login.password.c_str()) != MOSQ_ERR_SUCCESS) {
        log::warning("mqtt") << "Could not set username and password.";
        return false;
    }
    auto result { mosquitto_connect(m_mqtt, m_config.host.c_str(), m_config.port, m_config.keepalive) };
    if (result == MOSQ_ERR_SUCCESS) {
        m_connect_promise = std::promise<bool> {};
        m_connect_future = m_connect_promise.get_future();
        if ((m_connect_future.wait_for(m_config.timeout) == std::future_status::ready) && m_connect_future.get()) {
            return true;
        }
    } else {
        log::warning("mqtt") << "Could not connect: " << strerror(result);
    }
    return connect();
}

auto mqtt::disconnect() -> bool
{
    if (m_status != Status::Connected) {
        return true;
    }
    auto result { mosquitto_disconnect(m_mqtt) };
    if (result == MOSQ_ERR_SUCCESS) {
        set_status(Status::Disconnected);
        log::info("mqtt") << "Disconnected.";
        return true;
    }
    log::error("mqtt") << "Could not disconnect: " << result;
    return false;
}

void mqtt::set_status(Status status)
{
    m_status = status;
}

auto mqtt::publisher::publish(const std::string& content) -> bool
{
    return m_link->publish(m_topic, content);
}

auto mqtt::publisher::publish(const std::string& subtopic, const std::string& content) -> bool
{
    return m_link->publish(m_topic + '/' + subtopic, content);
}

auto mqtt::publisher::publish(const std::vector<std::string>& content) -> bool
{
    std::ostringstream stream {};
    for (const auto& string : content) {
        stream << string << '\n';
    }
    return m_link->publish(m_topic, stream.str());
}

auto mqtt::publisher::publish(const std::string& subtopic, const std::vector<std::string>& content) -> bool
{
    std::ostringstream stream {};
    for (const auto& string : content) {
        stream << string << '\n';
    }
    return m_link->publish(m_topic + '/' + subtopic, stream.str());
}

auto mqtt::publisher::get_publish_topic() const -> const std::string&
{
    return m_topic;
}

void mqtt::subscriber::emplace_callback(std::function<void(const message_t&)> callback)
{
    m_callback.emplace_back(std::move(callback));
}

void mqtt::subscriber::push_message(const message_t& message)
{
    for (auto& callback : m_callback) {
        callback(message);
    }
}

auto mqtt::subscriber::get_subscribe_topic() const -> const std::string&
{
    return m_topic;
}

auto mqtt::client_id() const -> std::string
{
    std::ostringstream out {};

    out << std::hex << std::hash<std::string> {}(m_config.login.username + m_station_id);

    return out.str();
}
} // namespace muonpi::link
