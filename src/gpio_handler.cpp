#include "muonpi/gpio_handler.h"


#include <cstring>

namespace muonpi {

gpio_handler::gpio_handler(const std::string& device, std::string consumer_name)
    : thread_runner { "gpiod" }
    , m_consumer { std::move(consumer_name) }
    , m_device { gpiod_chip_open(device.c_str())}

{
    if ( m_device == nullptr ) {
        log::error() << "error opening gpio chip '" << device <<"'";
        throw std::runtime_error{"error opening gpio chip"};
    }
    read_chip_info();
    start();
    m_callback_thread = std::thread{ [&](){
        while (m_run_callbacks) {
            std::mutex mx;
            std::unique_lock<std::mutex> lock{mx};

            m_events_available.wait(lock);

            if (!m_run_callbacks) {
                return;
            }


            while (!m_events.empty()) {
                m_event_rate.increase_counter();
                auto evt = m_events.front();
                m_events.pop();

                // Since no interrupts are registerred without callback, we always know that there is an associated callback
                for (auto& callback : m_callback.at(evt.pin).at(evt.edge)) {
                    callback(evt.pin, evt.edge, evt.timestamp);
                }
            }
            m_event_rate.step();

            const float timeout_us { std::clamp(s_m * m_event_rate.mean() + s_b, 0.0F, s_max_timeout) };

            m_inhibit_timeout = std::chrono::microseconds(static_cast<int>(timeout_us));
        }
    } };

}

gpio_handler::~gpio_handler()
{
    stop();

    for ( auto [gpio,line] : m_interrupt_lines ) {
        gpiod_line_release(line);
    }

    for ( auto [gpio,line] : m_output_lines ) {
        gpiod_line_release(line);
    }

    if ( m_device != nullptr ) {
        gpiod_chip_close( m_device );
    }
}

auto gpio_handler::set_pin_interrupt(pin_t pin, edge_t edge, bias_t bias, const callback_t& callback) -> bool
{
    auto it = m_callback.find(pin);
    if (it == m_callback.end()) {

        auto* line = allocate_interrupt_line(pin);

        if (gpiod_line_is_used(line)) {
            log::error()<<"Line "<<pin<<" is in use";
            throw std::runtime_error{"Line in use"};
        }

        auto flags { get_flags(bias) };

        int ret = -1;
        switch (edge) {
        case edge_t::Rising:
            ret = gpiod_line_request_rising_edge_events_flags( line, m_consumer.c_str(), flags );
            break;
        case edge_t::Falling:
            ret = gpiod_line_request_falling_edge_events_flags( line, m_consumer.c_str(), flags );
            break;
        case edge_t::Both:
            ret = gpiod_line_request_both_edges_events_flags( line, m_consumer.c_str(), flags );
            break;
        default:
            log::error()<<"Gpio input line " << pin << ": no valid edge defined";
            return false;
            break;
        }
        if ( ret < 0 ) {
            log::error()<<"Request gpio line " << pin << " for events failed";
            return false;
        }

        std::map<edge_t, std::vector<callback_t>> pin_callbacks {};
        if ((edge & edge_t::Falling) > 0) {
            pin_callbacks.emplace(edge_t::Falling, std::vector<callback_t>{callback});
        }
        if ((edge & edge_t::Rising) > 0) {
            pin_callbacks.emplace(edge_t::Rising, std::vector<callback_t>{callback});
        }

        m_callback.emplace(pin, pin_callbacks);

        if (m_autoreload) {
            reload_bulk_interrupt();
        }

        return true;
    }
    auto& [def_pin, callbacks] = *it;

    if ((edge & edge_t::Falling) > 0) {
        if (callbacks.find(edge_t::Falling) == callbacks.end()) {
            callbacks.emplace(edge_t::Falling, std::vector<callback_t>{callback});
        }
    }

    if ((edge & edge_t::Rising) > 0) {
        if (callbacks.find(edge_t::Rising) == callbacks.end()) {
            callbacks.emplace(edge_t::Rising, std::vector<callback_t>{callback});
        }
    }

    return true;
}

auto gpio_handler::set_pin_interrupt(const std::vector<std::tuple<pin_t, edge_t, bias_t>>& pins, const callback_t& callback) -> bool
{
    m_autoreload = false;
    return std::all_of(pins.begin(), pins.end(), [this, callback](const auto& it){
                            const auto& [pin, edge, bias] = it; return set_pin_interrupt(pin, edge, bias, callback);
    });
    m_autoreload = true;
    reload_bulk_interrupt();
}



auto gpio_handler::set_pin_output(pin_t pin, state_t initial_state, bias_t bias) -> std::function<bool(state_t)>
{
    auto* line = allocate_output_line(pin);

    if (gpiod_line_is_used(line)) {
        log::error()<<"Line "<<pin<<" is in use";
        throw std::runtime_error{"Line in use"};
    }


    int ret = gpiod_line_request_output_flags( line, m_consumer.c_str(), get_flags(bias), initial_state );

    if ( ret < 0 ) {
        log::error()<<"Request gpio line " << pin << " as output failed: " << std::strerror(errno);
        throw std::runtime_error{"Line request failed"};
    }

    return [&pin, this](state_t state){
        auto* line = allocate_output_line(pin);
        if (line == nullptr) {
            return false;
        }
        int ret = gpiod_line_set_value( line, static_cast<int>(state) );
        if ( ret < 0 ) {
            log::error()<<"Setting state of gpio line" << pin << "failed: " << std::strerror(errno);
            return false;
        }
        return true;
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


auto gpio_handler::get_chip_info() -> gpio_chip
{
    return m_chip;
}

void gpio_handler::read_chip_info()
{
    gpio_chip chip {};
    chip.name = gpiod_chip_name(m_device);
    chip.label = gpiod_chip_label(m_device);
    chip.num_lines = gpiod_chip_num_lines(m_device);

    for (std::size_t i { 0 }; i < chip.num_lines; i++) {
        auto* line = gpiod_chip_get_line(m_device, i);

        chip.lines.emplace_back(gpiod_line_name(line));

        gpiod_line_release(line);
    }

    m_chip = chip;
}

auto gpio_handler::step() -> int
{
    if ( m_inhibit ) {
        std::mutex mx;
        std::unique_lock<std::mutex> lock{mx};
        if (m_continue_inhibit.wait_for(lock, std::chrono::seconds{10} ) == std::cv_status::timeout) {
            return 0;
        }
    }
    std::this_thread::sleep_for( m_inhibit_timeout.load() );

    std::unique_lock<std::mutex> lock { m_mutex };


    const timespec timeout { 0, 100'000'000UL };
    gpiod_line_bulk event_bulk { };
    const int ret { gpiod_line_event_wait_bulk(&m_bulk_interrupt, &timeout, &event_bulk) };
    if ( ret < 0 ) {
        log::error()<<"Wait for gpio line events failed: " << ret;
        return 0;
    }

    if ( ret > 0 ) {
        for (std::size_t i { 0 }; i < event_bulk.num_lines; i++) {
            gpiod_line_event line_event { };
            const int result { gpiod_line_event_read( event_bulk.lines[i], &line_event) };
            if ( result != 0 ) {
                continue;
            }

            event_t evt {};
            evt.pin = gpiod_line_offset( event_bulk.lines[i] );

            switch (line_event.event_type) {
            case GPIOD_LINE_EVENT_RISING_EDGE:
                evt.edge = edge_t::Rising;
                break;
            case GPIOD_LINE_EVENT_FALLING_EDGE:
                evt.edge = edge_t::Falling;
                break;
            }

            evt.timestamp = time_t{std::chrono::seconds{line_event.ts.tv_sec} + std::chrono::nanoseconds{line_event.ts.tv_nsec}};

            m_events.emplace(evt);
        }
        m_events_available.notify_all();
    }
    return 0;
}

auto gpio_handler::post_run() -> int
{
    m_run_callbacks = false;
    m_events_available.notify_all();

    m_callback_thread.join();

    return 0;
}

auto gpio_handler::allocate_output_line(pin_t pin) -> gpiod_line*
{
    auto line_it = m_output_lines.find(pin);
    gpiod_line* line { nullptr };
    if (line_it == m_output_lines.end()) {
        line = gpiod_chip_get_line(m_device, pin);
    } else {
        line = line_it->second;
    }
    if ( line == nullptr ) {
        log::error()<<"error allocating gpio line " << pin;
        throw std::runtime_error{"error allocating gpio line"};
    }
    return line;
}

auto gpio_handler::allocate_interrupt_line(pin_t pin) -> gpiod_line*
{
    auto line_it = m_interrupt_lines.find(pin);
    gpiod_line* line { nullptr };
    if (line_it == m_interrupt_lines.end()) {
        line = gpiod_chip_get_line(m_device, pin);
    } else {
        line = line_it->second;
    }
    if ( line == nullptr ) {
        log::error()<<"error allocating gpio line " << pin;
        throw std::runtime_error{"error allocating gpio line"};
    }
    return line;
}

auto gpio_handler::get_flags(bias_t bias) -> int
{
    if ((bias & bias_t::OpenDrain) > 0) {
        return GPIOD_LINE_REQUEST_FLAG_OPEN_DRAIN;
    }
    if ((bias & bias_t::OpenSource) > 0) {
        return GPIOD_LINE_REQUEST_FLAG_OPEN_SOURCE;
    }
    if ((bias & bias_t::ActiveLow) > 0) {
        return GPIOD_LINE_REQUEST_FLAG_ACTIVE_LOW;
    }
    if ((bias & bias_t::PullDown) > 0) {
        return GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_DOWN;
    }
    if ((bias & bias_t::PullUp) > 0) {
        return GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP;
    }
    return GPIOD_LINE_REQUEST_FLAG_BIAS_DISABLE;
}

void gpio_handler::reload_bulk_interrupt()
{
    std::unique_lock<std::mutex> lock { m_mutex };

    gpiod_line_bulk_init( &m_bulk_interrupt );
    // rerequest bulk events
    for (auto& [gpio,line] : m_interrupt_lines) {
        gpiod_line_bulk_add( &m_bulk_interrupt, line );
    }
}

} // namespace muonpi
