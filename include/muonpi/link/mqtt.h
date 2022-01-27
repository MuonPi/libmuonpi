#ifndef MUONPI_LINK_MQTT_H
#define MUONPI_LINK_MQTT_H

#include "muonpi/global.h"

#include "muonpi/threadrunner.h"

#include <chrono>
#include <future>
#include <map>
#include <memory>
#include <queue>
#include <regex>
#include <string>
#include <utility>

#include <mosquitto.h>

namespace muonpi::error {

class LIBMUONPI_PUBLIC mqtt_could_not_subscribe : std::runtime_error {
public:
    mqtt_could_not_subscribe(const std::string& name, const std::string& reason)
        : std::runtime_error { "Could not subscribe to mqtt topic '" + name + "': " + reason }
    {
    }
};

class LIBMUONPI_PUBLIC mqtt_could_not_publish : std::runtime_error {
public:
    mqtt_could_not_publish(const std::string& name, const std::string& reason)
        : std::runtime_error { "Could not publish mqtt topic '" + name + "': " + reason }
    {
    }
};

class LIBMUONPI_PUBLIC config_option_not_found : std::runtime_error {
public:
    explicit config_option_not_found(const std::string& name)
        : std::runtime_error { "Could not find configuration option '" + name + "'" }
    {
    }
};

} // namespace muonpi::error

namespace muonpi::link {

/**
 * @brief The mqtt class. Connects to a mqtt server and offers publish and subscribe methods.
 */
class LIBMUONPI_PUBLIC mqtt : public thread_runner {
public:
    enum class Status {
        Invalid,
        Connected,
        Disconnected,
        Connecting,
        Error
    };

    struct configuration {
        std::string host {};
        int port { 1883 };
        struct login_t {
            std::string username {};
            std::string password {};
        } login;
        std::size_t max_retries { 10 };
        std::chrono::seconds timeout { 3 };
        int keepalive { 60 };
    };

    struct message_t {
        message_t() = default;
        message_t(std::string a_topic, std::string a_content)
            : topic { std::move(a_topic) }
            , content { std::move(a_content) }
        {
        }
        std::string topic {};
        std::string content {};
    };

    /**
     * @brief The publisher class. Only gets instantiated from within the mqtt class.
     */
    class publisher {
    public:
        publisher(mqtt* link, std::string topic)
            : m_link { link }
            , m_topic { std::move(topic) }
        {
        }

        /**
         * @brief publish Publish a message
         * @param content The content to send
         * @return true if the sending was successful
         */
        auto publish(const std::string& content) -> bool;

        /**
         * @brief publish Publish a message
         * @param subtopic Subtopic to add to the basetopic specified in the constructor
         * @param content The content to send
         * @return true if the sending was successful
         */
        auto publish(const std::string& subtopic, const std::string& content) -> bool;

        /**
         * @brief publish Publish a number of messages combined to one.
         * @param content Vector with the content to send
         * @return true if the sending was successful
         */
        auto publish(const std::vector<std::string>& content) -> bool;

        /**
         * @brief publish Publish a number of messages combined to one.
         * @param subtopic Subtopic to add to the basetopic specified in the constructor
         * @param content Vector with the content to send
         * @return true if the sending was successful
         */
        auto publish(const std::string& subtopic, const std::vector<std::string>& content) -> bool;

        /**
         * @brief get_publish_topic Gets the topic under which the publisher publishes messages
         * @return a std::string containing the publish topic
         */
        [[nodiscard]] auto get_publish_topic() const -> const std::string&;

        publisher() = default;

    private:
        friend class mqtt;

        mqtt* m_link { nullptr };
        std::string m_topic {};
    };

    /**
     * @brief The subscriber class. Only gets instantiated from within the mqtt class.
     */
    class subscriber {
    public:
        subscriber(mqtt* link, std::string topic)
            : m_link { link }
            , m_topic { std::move(topic) }
        {
        }

        ~subscriber()
        {
            m_link->unsubscribe(m_topic);
        }

        subscriber() = default;

        void emplace_callback(std::function<void(const message_t&)> callback);

        /**
         * @brief get_subscribe_topic Gets the topic the subscriber subscribes to
         * @return a std::string containing the subscribed topic
         */
        [[nodiscard]] auto get_subscribe_topic() const -> const std::string&;

    private:
        friend class mqtt;

        /**
         * @brief push_message Only called from within the mqtt class
         * @param message The message to push into the queue
         */
        void push_message(const message_t& message);

        mqtt* m_link { nullptr };
        std::string m_topic {};
        std::vector<std::function<void(const message_t&)>> m_callback;
    };

    /**
     * @brief mqtt
     * @param config The configuration to use
     */
    mqtt(configuration config, std::string station_id, const std::string& name = "muon::mqtt");

    explicit mqtt(std::string station_id, const std::string& name = "muon::mqtt");

    ~mqtt() override;

    /**
     * @brief publish Create a publisher callback object
     * @param topic The topic under which the publisher sends messages
     */
    [[nodiscard]] auto publish(const std::string& topic) -> publisher&;

    /**
     * @brief subscribe Create a subscriber callback object
     * @param topic The topic to subscribe to. See mqtt for wildcards.
     */
    [[nodiscard]] auto subscribe(const std::string& topic) -> subscriber&;

    /**
     * @brief wait_for Wait for a designated time until the status changes to the one set as the parameter
     * @param status The status to wait for
     * @param duration The duration to wait for as a maximum
     */
    [[nodiscard]] auto wait_for(Status status, std::chrono::milliseconds duration = std::chrono::seconds { 5 }) -> bool;

protected:
    /**
     * @brief pre_run Reimplemented from thread_runner
     * @return 0 if the thread should start
     */
    [[nodiscard]] auto pre_run() -> int override;
    /**
     * @brief step Reimplemented from thread_runner
     * @return 0 if the thread should continue running
     */
    [[nodiscard]] auto step() -> int override;
    /**
     * @brief post_run Reimplemented from thread_runner
     * @return The return value of the thread loop
     */
    [[nodiscard]] auto post_run() -> int override;

    void on_stop() override;

private:
    /**
     * @brief set_status Set the status for this mqtt
     * @param status The new status
     */
    void set_status(Status status);

    [[nodiscard]] auto publish(const std::string& topic, const std::string& content) -> bool;

    /**
     * @brief unsubscribe Unsubscribe from a specific topic
     * @param topic The topic string to unsubscribe from
     */
    void unsubscribe(const std::string& topic);

    /**
     * @brief connects to the Server synchronuously. This method blocks until it is connected.
     * @return true if the connection was successful
     */
    [[nodiscard]] auto connect() -> bool;

    /**
     * @brief disconnect Disconnect from the server
     * @return true if the disconnect was successful
     */
    auto disconnect() -> bool;

    [[nodiscard]] auto check_connection() -> bool;

    auto p_subscribe(const std::string& topic) -> bool;

    /**
     * @brief init Initialise the mosquitto object. This is necessary since the mosquitto_lib_init() needs to be called before mosquitto_new().
     * @param client_id The client_id to use
     */
    [[nodiscard]] inline auto init(const char* client_id) -> mosquitto*
    {
        mosquitto_lib_init();
        return mosquitto_new(client_id, true, this);
    }

    configuration m_config {};
    std::string m_station_id {};
    mosquitto* m_mqtt { nullptr };

    Status m_status { Status::Invalid };

    std::map<std::string, std::unique_ptr<publisher>> m_publishers {};
    std::map<std::string, std::unique_ptr<subscriber>> m_subscribers {};

    std::promise<bool> m_connect_promise {};
    std::future<bool> m_connect_future {};
    std::condition_variable m_connect_condition {};

    std::size_t m_tries { 0 };

    /**
     * @brief callback_connected Gets called by mosquitto client
     * @param result The status code from the callback
     */
    void callback_connected(int result);

    /**
     * @brief callback_disconnected Gets called by mosquitto client
     * @param result The status code from the callback
     */
    void callback_disconnected(int result);

    /**
     * @brief callback_message Gets called by mosquitto client in the case of an arriving message
     * @param message A const pointer to the received message
     */
    void callback_message(const mosquitto_message* message);

    /**
     * @brief client_id Creates a client_id from the username and the station id.
     * This hashes the concatenation of the two fields.
     * @return The client id as string
     */
    [[nodiscard]] auto client_id() const -> std::string;

    friend void wrapper_callback_connected(mosquitto* mqtt, void* object, int result);
    friend void wrapper_callback_disconnected(mosquitto* mqtt, void* object, int result);
    friend void wrapper_callback_message(mosquitto* mqtt, void* object, const mosquitto_message* message);
};

} // namespace muonpi::link

#endif // MUONPI_LINK_MQTT_H
