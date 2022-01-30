#include "muonpi/gpio_handler.h"

#include <cstring>

namespace muonpi {

gpio_handler::gpio_handler(const std::string& device, std::string consumer_name)
    : thread_runner { "gpiod", true }
    , m_consumer { std::move(consumer_name) }
    , m_device { gpiod_chip_open(device.c_str()) }

{
    if (m_device == nullptr) {
        log::error() << "error opening gpio chip '" << device << "'";
        throw std::runtime_error { "error opening gpio chip" };
    }
    read_chip_info();
    start();
}

gpio_handler::~gpio_handler()
{
    stop();

    for (auto [gpio, line] : m_interrupt_lines) {
        gpiod_line_release(line);
    }

    for (auto [gpio, line] : m_io_lines) {
        gpiod_line_release(line);
    }

    if (m_device != nullptr) {
        gpiod_chip_close(m_device);
    }
}

auto gpio_handler::set_pin_interrupt(const gpio::settings_t& settings, const gpio::callback_t& callback) -> bool
{
    auto it = m_callback.find(settings.pin);
    if (it == m_callback.end()) {

        auto* line = allocate_interrupt_line(settings.pin);

        if (gpiod_line_is_used(line)) {
            log::error() << "Line " << settings.pin << " is in use";
            throw std::runtime_error { "Line in use" };
        }

        auto flags { get_flags(settings.bias) };

        int ret = -1;
        switch (settings.edge) {
        case gpio::edge_t::Rising:
            ret = gpiod_line_request_rising_edge_events_flags(line, m_consumer.c_str(), flags);
            break;
        case gpio::edge_t::Falling:
            ret = gpiod_line_request_falling_edge_events_flags(line, m_consumer.c_str(), flags);
            break;
        case gpio::edge_t::Both:
            ret = gpiod_line_request_both_edges_events_flags(line, m_consumer.c_str(), flags);
            break;
        default:
            log::error() << "Gpio input line " << settings.pin << ": no valid edge defined";
            return false;
            break;
        }
        if (ret < 0) {
            log::error() << "Request gpio line " << settings.pin << " for events failed";
            return false;
        }

        std::map<gpio::edge_t, std::vector<gpio::callback_t>> pin_callbacks {};
        if ((settings.edge & gpio::edge_t::Falling) > 0) {
            pin_callbacks.emplace(gpio::edge_t::Falling, std::vector<gpio::callback_t> { callback });
        }
        if ((settings.edge & gpio::edge_t::Rising) > 0) {
            pin_callbacks.emplace(gpio::edge_t::Rising, std::vector<gpio::callback_t> { callback });
        }

        m_callback.emplace(settings.pin, pin_callbacks);
        m_interrupt_condition.notify_all();

        m_bulk_dirty = true;

        log::debug() << "Registered event callback for pin " << settings.pin << " '" << m_chip.lines.at(settings.pin).name << "'";
        return true;
    }
    auto& [def_pin, callbacks] = *it;

    if ((settings.edge & gpio::edge_t::Falling) > 0) {
        if (callbacks.find(gpio::edge_t::Falling) == callbacks.end()) {
            callbacks.emplace(gpio::edge_t::Falling, std::vector<gpio::callback_t> { callback });
        }
    }

    if ((settings.edge & gpio::edge_t::Rising) > 0) {
        if (callbacks.find(gpio::edge_t::Rising) == callbacks.end()) {
            callbacks.emplace(gpio::edge_t::Rising, std::vector<gpio::callback_t> { callback });
        }
    }

    return true;
}

auto gpio_handler::set_pin_interrupt(const gpio::pins_t& pins, const gpio::callback_t& callback) -> bool
{
    return std::all_of(pins.begin(), pins.end(), [this, callback](const auto& settings) {
        return set_pin_interrupt(settings, callback);
    });
}

auto gpio_handler::set_pin_output(gpio::pin_t pin, gpio::state_t initial_state, gpio::bias_t bias) -> std::function<bool(gpio::state_t)>
{
    auto* line = allocate_io_line(pin);

    if (gpiod_line_is_used(line)) {
        log::error() << "Line " << pin << " is in use";
        throw std::runtime_error { "Line in use" };
    }

    int ret = gpiod_line_request_output_flags(line, m_consumer.c_str(), get_flags(bias), static_cast<int>(initial_state));

    if (ret < 0) {
        log::error() << "Request gpio line " << pin << " as output failed: " << std::strerror(errno);
        throw std::runtime_error { "Line request failed" };
    }

    return [pin, this](gpio::state_t state) {
        auto* l = allocate_io_line(pin);
        if (l == nullptr) {
            return false;
        }
        int r = gpiod_line_set_value(l, static_cast<int>(state));
        if (r < 0) {
            log::error() << "Setting state of gpio line " << pin << " failed: " << std::strerror(errno);
            return false;
        }
        return true;
    };
}

auto gpio_handler::get_pin_input(gpio::pin_t pin, gpio::bias_t bias) -> std::function<gpio::state_t()>
{
    auto* line = allocate_io_line(pin);

    if (gpiod_line_is_used(line)) {
        log::error() << "Line " << pin << " is in use";
        throw std::runtime_error { "Line in use" };
    }

    int ret = gpiod_line_request_input_flags(line, m_consumer.c_str(), get_flags(bias));

    if (ret < 0) {
        log::error() << "Request gpio line " << pin << " as input failed: " << std::strerror(errno);
        throw std::runtime_error { "Line request failed" };
    }

    return [pin, this]() {
        auto* l = allocate_io_line(pin);
        if (l == nullptr) {
            return gpio::state_t { gpio::state_t::Undefined };
        }
        int r = gpiod_line_get_value(l);
        if (r < 0) {
            log::error() << "Setting state of gpio line " << pin << " failed: " << std::strerror(errno);
            return gpio::state_t { gpio::state_t::Undefined };
        }
        return static_cast<gpio::state_t>(r);
    };
}

void gpio_handler::start_inhibit()
{
    m_inhibit = true;
}

void gpio_handler::end_inhibit()
{
    m_inhibit = false;
    m_continue_inhibit.notify_all();
}

auto gpio_handler::get_chip_info() -> gpio::chip_info
{
    return m_chip;
}

void gpio_handler::read_chip_info()
{
    gpio::chip_info chip {};
    chip.name = gpiod_chip_name(m_device);
    chip.label = gpiod_chip_label(m_device);
    chip.num_lines = gpiod_chip_num_lines(m_device);

    for (std::size_t i { 0 }; i < chip.num_lines; i++) {
        auto* line = gpiod_chip_get_line(m_device, i);
        const auto* name = gpiod_line_name(line);
        const auto* consumer = gpiod_line_consumer(line);
        gpio::chip_info::line_t l {};

        if (name == nullptr) {
            l.name = "";
        } else {
            l.name = name;
        }

        if (consumer == nullptr) {
            l.consumer = "";
        } else {
            l.consumer = consumer;
        }

        chip.lines.emplace_back(l);

        gpiod_line_release(line);
    }

    m_chip = chip;
}

auto gpio_handler::custom_run() -> int
{
    while (!m_quit) {
        if (m_bulk_dirty) {
            reload_bulk_interrupt();
        }

        const timespec timeout { 1, 0 };
        gpiod_line_bulk event_bulk {};
        const int ret { gpiod_line_event_wait_bulk(&m_bulk_interrupt, &timeout, &event_bulk) };

        if (ret > 0) {

            for (std::size_t i { 0 }; i < event_bulk.num_lines; i++) {
                gpiod_line_event line_event {};
                const int result { gpiod_line_event_read(event_bulk.lines[i], &line_event) };
                if (result != 0) {
                    continue;
                }

                m_events.emplace(private_event { gpiod_line_offset(event_bulk.lines[i]), line_event });
            }
            m_events_available.notify_all();
        } else if (ret < 0) {
            log::error() << "Wait for gpio line events failed: " << ret;
        }
    }
    return 0;
}

auto gpio_handler::pre_run() -> int
{
    std::mutex mx {};
    std::unique_lock<std::mutex> lock { mx };

    if (m_interrupt_condition.wait_for(lock, std::chrono::seconds { 5 }) == std::cv_status::timeout && m_interrupt_lines.empty()) {
        return 1;
    }

    m_callback_thread = std::thread { [&]() {
        while (!m_quit) {
            std::mutex cb_mx;
            std::unique_lock<std::mutex> cb_lock { cb_mx };

            m_events_available.wait(cb_lock);

            if (m_quit) {
                return;
            }

            if (m_inhibit) {
                while (!m_events.empty()) {
                    m_events.pop();
                }
                continue;
            }

            while (!m_events.empty()) {
                auto event = m_events.front();
                m_events.pop();
                gpio::event_t evt {};
                evt.pin = event.pin;

                switch (event.evt.event_type) {
                case GPIOD_LINE_EVENT_RISING_EDGE:
                    evt.edge = gpio::edge_t::Rising;
                    break;
                case GPIOD_LINE_EVENT_FALLING_EDGE:
                    evt.edge = gpio::edge_t::Falling;
                    break;
                }

                evt.time = gpio::time_t { std::chrono::seconds { event.evt.ts.tv_sec } + std::chrono::nanoseconds { event.evt.ts.tv_nsec } };

                m_event_rate.increase_counter();

                // Since no interrupts are registerred without callback, we always know that there is an associated callback
                for (auto& callback : m_callback.at(evt.pin).at(evt.edge)) {
                    callback(evt);
                }
            }

            if (m_event_rate.step()) {
                const float timeout_us { std::clamp(s_m * m_event_rate.mean() + s_b, 0.0F, s_max_timeout) };

                m_inhibit_timeout = std::chrono::microseconds(static_cast<int>(timeout_us));
            }
        }
    } };

    reload_bulk_interrupt();
    return 0;
}

auto gpio_handler::post_run() -> int
{
    m_events_available.notify_all();

    m_callback_thread.join();

    return 0;
}

auto gpio_handler::allocate_io_line(gpio::pin_t pin) -> gpiod_line*
{
    auto line_it = m_io_lines.find(pin);
    gpiod_line* line { nullptr };
    if (line_it == m_io_lines.end()) {
        line = gpiod_chip_get_line(m_device, pin);
        m_io_lines.emplace(pin, line);
    } else {
        line = line_it->second;
    }
    if (line == nullptr) {
        log::error() << "error allocating gpio line " << pin;
        throw std::runtime_error { "error allocating gpio line" };
    }
    return line;
}

auto gpio_handler::allocate_interrupt_line(gpio::pin_t pin) -> gpiod_line*
{
    auto line_it = m_interrupt_lines.find(pin);
    gpiod_line* line { nullptr };
    if (line_it == m_interrupt_lines.end()) {
        line = gpiod_chip_get_line(m_device, pin);
        m_interrupt_lines.emplace(pin, line);
    } else {
        line = line_it->second;
    }
    if (line == nullptr) {
        log::error() << "error allocating gpio line " << pin;
        throw std::runtime_error { "error allocating gpio line" };
    }
    return line;
}

auto gpio_handler::get_flags(gpio::bias_t bias) -> int
{
    if ((bias & gpio::bias_t::OpenDrain) > 0) {
        return GPIOD_LINE_REQUEST_FLAG_OPEN_DRAIN;
    }
    if ((bias & gpio::bias_t::OpenSource) > 0) {
        return GPIOD_LINE_REQUEST_FLAG_OPEN_SOURCE;
    }
    if ((bias & gpio::bias_t::ActiveLow) > 0) {
        return GPIOD_LINE_REQUEST_FLAG_ACTIVE_LOW;
    }
    return 0;
}

void gpio_handler::reload_bulk_interrupt()
{
    gpiod_line_bulk_init(&m_bulk_interrupt);
    for (auto& [gpio, line] : m_interrupt_lines) {
        gpiod_line_bulk_add(&m_bulk_interrupt, line);
    }
    m_bulk_dirty = false;
}

} // namespace muonpi
