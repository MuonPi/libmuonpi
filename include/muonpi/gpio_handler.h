#ifndef GPIO_HANDLER_H
#define GPIO_HANDLER_H

#include "muonpi/analysis/ratemeasurement.h"
#include "muonpi/global.h"
#include "muonpi/threadrunner.h"

#include "muonpi/log.h"

#include <gpiod.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <vector>

namespace muonpi {

namespace gpio {
    /**
     * @brief The gpio_chip struct.
     * Information about a gpio chip.
     */
    struct chip_info {
        std::string name; ///<! The name of the chip as present in the kernel
        std::string label; ///<! The label of the chip as present in the kernel
        std::size_t num_lines; ///<! The number of lines present in the chip
        struct line_t {
            std::string name;
            std::string consumer;
        };
        std::vector<line_t> lines; ///<! The names of all lines reported in the chip
    };

    /**
     * @brief The bias_t enum.
     * bias settings for gpio pins.
     */
    enum bias_t : std::uint8_t {
        Disabled = 0x00,
        PullDown = 0x01,
        PullUp = 0x02,
        ActiveLow = 0x04,
        OpenDrain = 0x08,
        OpenSource = 0x10
    };

    /**
     * @brief The edge_t enum.
     * The type of edge detection for interrupts
     */
    enum edge_t {
        Rising = 0x01,
        Falling = 0x02,
        Both = Rising | Falling
    };

    /**
     * @brief The state_t enum.
     * The state of an output pin.
     */
    enum state_t : int {
        Low = 0,
        High = 1
    };

    // +++ convencience definitions
    using pin_t = unsigned int;
    using time_t = std::chrono::system_clock::time_point;
    struct settings_t {
        gpio::pin_t pin;
        gpio::edge_t edge;
        gpio::bias_t bias;
    };

    using pins_t = std::vector<settings_t>;
    // --- convencience definitions

    /**
     * @brief The event_t struct.
     * Convenience struct for the event queue
     */
    struct event_t {
        gpio::pin_t pin {}; ///<! The pin offset of the event
        gpio::edge_t edge; ///<! The type of detected edge
        time_t time; ///<! The timestamp of the event
    };

    using callback_t = std::function<void(event_t)>;
} // namespace gpio

/**
 * @brief The gpio_handler class.
 * Starts two threads: One to handle the actual gpio interface and
 * one to handle the callback invokation as well as rate limiting through timeout inhibition.
 * The callbacks do not delay the gpio reading thread, though they should still not take too long to excute.
 */
class LIBMUONPI_PUBLIC gpio_handler : public thread_runner {
public:
    /**
     * @brief gpio_handler
     * @param device The device file to use
     * @param consumer_name The name of the consumer which should be given to the kernel
     */
    gpio_handler(const std::string& device, std::string consumer_name);

    ~gpio_handler() override;

    /**
     * @brief set_pin_interrupt Enable a callback for one specific pin.
     * @param settings The pin settings.
     * @param callback The callback to invoke when the event is detected
     * @return True when the event has been registered
     */
    [[nodiscard]] auto set_pin_interrupt(const gpio::settings_t& settings, const gpio::callback_t& callback) -> bool;

    /**
     * @brief set_pin_interrupt Add a callback to a number of pin definitions
     * @param settings A vector containing all pin settings.
     * @param callback The callback to invoke on each event
     * @return True when all events have been registered
     */
    [[nodiscard]] auto set_pin_interrupt(const gpio::pins_t& settings, const gpio::callback_t& callback) -> bool;

    /**
     * @brief set_pin_output Configure a pin to function as an output pin
     * @param pin The pin number to configure
     * @param initial_state The initial state of the pin
     * @param bias The bias settings for the pin
     * @return A lambda which can be used to set the state of the pin.
     */
    [[nodiscard]] auto set_pin_output(gpio::pin_t pin, gpio::state_t initial_state, gpio::bias_t bias) -> std::function<bool(gpio::state_t)>;

    /**
     * @brief start_inhibit Stop all event processing.
     */
    void start_inhibit();

    /**
     * @brief end_inhibit Resume event processing
     */
    void end_inhibit();

    /**
     * @brief get_chip_info Get information about the currently selected chip
     * @return
     */
    [[nodiscard]] auto get_chip_info() -> gpio::chip_info;

protected:
    /**
     * @brief custom_run Reimplemented from thread_runner. Executed continuously
     * @return 0 for success, if nonzero the thread stops
     */
    [[nodiscard]] auto custom_run() -> int override;

    /**
     * @brief post_run Reimplemented from thread_runner. Executed after the thread loop is stopped.
     * @return
     */
    [[nodiscard]] auto post_run() -> int override;

    /**
     * @brief pre_run Reimplemented from thread_runner. Executed before the thread loop is started.
     * @return
     */
    [[nodiscard]] auto pre_run() -> int override;

private:
    /**
     * @brief read_chip_info Read the chip information
     * @return
     */
    void read_chip_info();

    /**
     * @brief allocate_output_line Allocates a line for output, or returns the line if already allocated
     * @param pin the pin number to use
     */
    [[nodiscard]] auto allocate_io_line(gpio::pin_t pin) -> gpiod_line*;

    /**
     * @brief allocate_interrupt_line Allocates a line for interrupt, or returns the line if already allocated
     * @param pin the pin number to use
     */
    [[nodiscard]] auto allocate_interrupt_line(gpio::pin_t pin) -> gpiod_line*;

    /**
     * @brief get_flags get setting flags from the bias settings
     * @param bias The bias setting to use
     * @return The flag from libgpiod associated with the bias setting
     */
    [[nodiscard]] static auto get_flags(gpio::bias_t bias) -> int;

    /**
     * @brief reload_bulk_interrupt Combine all interrupt lanes to one bulk interrupt object
     */
    void reload_bulk_interrupt();

    std::atomic<bool> m_bulk_dirty { true };

    std::map<gpio::pin_t, std::map<gpio::edge_t, std::vector<gpio::callback_t>>> m_callback {}; ///<! All registered callbacks

    std::atomic<bool> m_inhibit { false }; ///<! Inhibit the event processing execution

    std::condition_variable m_continue_inhibit {}; ///<! Continue to inhibit. Notify to stop inhibition

    std::string m_consumer; ///<! The consumer identifier to use

    gpiod_chip* m_device { nullptr }; ///<! The device pointer

    std::map<gpio::pin_t, gpiod_line*> m_interrupt_lines {}; ///<! Registered interrupt lines
    gpiod_line_bulk m_bulk_interrupt {}; ///<! The bulk interrupt object
    std::map<gpio::pin_t, gpiod_line*> m_io_lines {}; ///<! Registered I/O lines

    std::condition_variable m_events_available {}; ///<! Condition variable for thread synchronisation

    std::thread m_callback_thread {}; ///<! The thread which handles all callbacks

    std::condition_variable m_interrupt_condition {};

    rate_measurement<float> m_event_rate { 100, std::chrono::seconds { 6 } }; ///<! Rate measurement object for the incoming event rate

    std::atomic<std::chrono::system_clock::duration> m_inhibit_timeout { std::chrono::microseconds { 0 } }; ///<! dynamic timeout for the inhibition time

    struct private_event {
        gpio::pin_t pin;
        gpiod_line_event evt;
    };

    std::queue<private_event> m_events {}; ///<! queue with all current events

    gpio::chip_info m_chip {};

    constexpr static float s_min_rate { 10.0F }; ///<! Minimum rate in Hz
    constexpr static float s_max_rate { 100.0F }; ///<! Maximum rate in Hz
    constexpr static float s_max_timeout { 100'000.0F }; ///<! Maximum timeout in us

    constexpr static float s_b { s_max_timeout * s_min_rate / (s_min_rate - s_max_rate) }; ///<! m*x+b
    constexpr static float s_m { -s_max_timeout / (s_min_rate - s_max_rate) }; ///<! m*x+b
};
} // namespace muonpi

#endif // GPIO_HANDLER_H
