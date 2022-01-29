#include "muonpi/threadrunner.h"

#include <utility>

#include "muonpi/log.h"
#include "muonpi/scopeguard.h"

namespace muonpi {

thread_runner::thread_runner(std::string name, bool use_custom_run)
    : m_use_custom_run { use_custom_run }
    , m_name { std::move(name) }
{
}

thread_runner::~thread_runner() = default;

void thread_runner::stop(int exit_code)
{
    m_run = false;
    m_quit = true;
    m_exit_code = exit_code;
    m_condition.notify_all();
    on_stop();
}

void thread_runner::join()
{
    if ((m_thread != nullptr) && m_thread->joinable()) {
        m_thread->join();
        m_thread.reset();
    }
    if (m_run_future.valid()) {
        m_run_future.wait();
    }
}

auto thread_runner::step() -> int
{
    return 0;
}

auto thread_runner::pre_run() -> int
{
    return 0;
}

auto thread_runner::post_run() -> int
{
    return 0;
}

auto thread_runner::custom_run() -> int
{
    return 0;
}

void thread_runner::on_stop()
{
}

auto thread_runner::wait() -> int
{
    if (!m_run_future.valid()) {
        return -1;
    }
    join();
    return m_run_future.get();
}

auto thread_runner::state() -> State
{
    return m_state;
}

auto thread_runner::run() -> int
{
    set_state(State::Initialising);
    bool clean { false };
    const scope_guard state_guard { [&] {
        if (clean) {
            set_state(State::Stopped);
        } else {
            set_state(State::Error);
        }
    } };

    try {
        log::debug("thread") << "Starting '" << m_name << '\'';
        const auto pre_result { pre_run() };
        if (pre_result != 0) {
            return pre_result;
        }

        if ((m_thread != nullptr)) {
            auto handle { m_thread->native_handle() };
            const auto result { pthread_setname_np(handle, m_name.c_str()) };
            if (result != 0) {
            }
        }
        set_state(State::Running);
        if (m_use_custom_run) {
            int result { custom_run() };
            if (result != 0) {
                log::warning("thread") << "'" << m_name << "' Stopped.";
                m_exit_code = result;
            }
        } else {
            while (m_run) {
                int result { step() };
                if (result != 0) {
                    log::warning("thread") << "'" << m_name << "' Stopped.";
                    m_exit_code = result;
                    break;
                }
            }
        }
        set_state(State::Finalising);
        log::debug("thread") << "Stopping '" << m_name << '\'';

        clean = (m_exit_code == 0);

        return post_run() + m_exit_code;
    } catch (std::exception& e) {
        log::error("thread") << "'" << m_name << "' Got an uncaught exception: " << e.what();
        return -1;
    } catch (...) {
        log::error("thread") << "'" << m_name << "' Got an uncaught exception.";
        return -1;
    }
}

void thread_runner::exec()
{

    std::promise<int> promise {};
    m_run_future = promise.get_future();
    int value = run();
    promise.set_value(value);
}

void thread_runner::finish()
{
    stop();
    join();
}

auto thread_runner::name() -> std::string
{
    return m_name;
}

auto thread_runner::state_string() -> std::string
{
    switch (m_state) {
    case State::Error:
        return "Error";
    case State::Stopped:
        return "Stopped";
    case State::Initial:
        return "Initial";
    case State::Initialising:
        return "Initialising";
    case State::Running:
        return "Running";
    case State::Finalising:
        return "Finalising";
    }
    return {};
}

void thread_runner::start()
{
    if ((m_state > State::Initial) || (m_thread != nullptr)) {
        log::info("thread") << "'" << m_name << "' already running, refusing to start.";
        return;
    }
    m_thread = std::make_unique<std::thread>(&thread_runner::exec, this);
}

void thread_runner::start_synchronuos()
{
    if (m_state > State::Initial) {
        return;
    }
    exec();
}

void thread_runner::set_state(State state)
{
    m_state = state;
    m_state_condition.notify_all();
}

auto thread_runner::wait_for(State state, std::chrono::milliseconds duration) -> bool
{
    std::chrono::steady_clock::time_point start { std::chrono::steady_clock::now() };
    auto waited { std::chrono::steady_clock::now() - start };
    while (waited < duration) {
        if (m_state == state) {
            return true;
        }
        std::mutex mx;
        std::unique_lock<std::mutex> lock { mx };
        if (m_state_condition.wait_for(lock, duration - waited) == std::cv_status::timeout) {
            return false;
        }
        waited = std::chrono::steady_clock::now() - start;
    }
    return false;
}

} // namespace muonpi
