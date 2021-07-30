#ifndef GPIO_HANDLER_H
#define GPIO_HANDLER_H

#include "muonpi/global.h"
#include "muonpi/threadrunner.h"
#include "muonpi/analysis/ratemeasurement.h"

#include "muonpi/log.h"

#include <gpiod.h>

#include <map>
#include <vector>
#include <functional>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace muonpi {

class LIBMUONPI_PUBLIC gpio_handler : public thread_runner
{
public:
    enum bias_t : std::uint8_t {
        Disabled = 0x00,
        PullDown = 0x01,
        PullUp = 0x02,
        ActiveLow = 0x04,
        OpenDrain = 0x08,
        OpenSource = 0x10
    };

    enum edge_t {
        Rising = 0x01,
        Falling = 0x02,
        Both = Rising | Falling
    };

    enum state_t : int {
        Low = 0,
        High = 1
    };

    using pin_t = unsigned int;
    using time_t = std::chrono::system_clock::time_point;
    using callback_t = std::function<void(pin_t, edge_t, time_t)>;

    gpio_handler(const std::string& device, std::string consumer_name);
    ~gpio_handler();

    [[nodiscard]] auto set_pin_interrupt(pin_t pin, edge_t edge, bias_t bias, const callback_t& callback) -> bool;
    [[nodiscard]] auto set_pin_interrupt(const std::vector<std::tuple<pin_t, edge_t, bias_t>>& pins, const callback_t& callback) -> bool;

    [[nodiscard]] auto set_pin_output(pin_t pin, state_t initial_state, bias_t bias) -> std::function<bool(state_t)>;


    void start_inhibit();
    void end_inhibit();

protected:
    [[nodiscard]] auto step() -> int override;
    [[nodiscard]] auto post_run() -> int override;

private:
    [[nodiscard]] auto allocate_output_line(pin_t pin) -> gpiod_line*;
    [[nodiscard]] auto allocate_interrupt_line(pin_t pin) -> gpiod_line*;

    [[nodiscard]] static auto get_flags(bias_t bias) -> int;

    void reload_bulk_interrupt();

    std::map<pin_t, std::map<edge_t, std::vector<callback_t>>> m_callback{};

    std::chrono::system_clock::time_point m_startup { std::chrono::system_clock::now() };

    std::atomic<bool> m_inhibit { false };
    std::condition_variable m_continue_inhibit {};

    std::string m_consumer;

    gpiod_chip* m_device { nullptr };

    std::map<pin_t, gpiod_line*> m_interrupt_lines { };
    gpiod_line_bulk m_bulk_interrupt{};
    std::map<pin_t, gpiod_line*> m_output_lines { };

    std::condition_variable m_events_available {};

    std::thread m_callback_thread {};

    std::atomic<bool> m_run_callbacks { true };

    rate_measurement<float> m_event_rate {100, std::chrono::seconds{6} };

    std::atomic<std::chrono::system_clock::duration> m_inhibit_timeout { std::chrono::microseconds{1} };

    struct event_t {
        pin_t pin;
        edge_t edge;
        time_t timestamp;
    };

    std::queue<event_t> m_events {};

    std::mutex m_mutex;

    constexpr static float s_min_rate { 10.0F }; //<! Minimum rate in Hz
    constexpr static float s_max_rate { 100.0F }; //<! Maximum rate in Hz
    constexpr static float s_max_timeout { 100'000.0F }; //<! Maximum timeout in us

    constexpr static float s_b { s_max_timeout * s_min_rate / (s_min_rate - s_max_rate) }; //<! m*x+b
    constexpr static float s_m { - s_max_timeout / (s_min_rate - s_max_rate) }; //<! m*x+b
};
}

#endif // GPIO_HANDLER_H
