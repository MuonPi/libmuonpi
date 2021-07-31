#ifndef THREADRUNNER_H
#define THREADRUNNER_H

#include "muonpi/global.h"

#include <atomic>
#include <future>

namespace muonpi {

/**
 * @brief The thread_runner class. Inherit from this to get a class which has an internal main loop.
 * If an instance of this class is created without inheriting from it, the main loop will stop immediatly per default.
 */
class LIBMUONPI_PUBLIC thread_runner {
public:
    enum class State {
        Error,
        Stopped,
        Initial,
        Initialising,
        Running,
        Finalising
    };
    thread_runner(std::string name, bool use_custom_run = false);

    /**
     * @brief ~thread_runner Stops the thread and waits for it to finish.
     */
    virtual ~thread_runner();

    /**
     * @brief stop Tells the main loop to finish
     */
    void stop(int exit_code = 0);

    /**
     * @brief join waits for the thread to finish
     */
    void join();

    /**
     * @brief wait Wait for the main loop to finish
     * @return the return value of the main loop
     */
    [[nodiscard]] auto wait() -> int;

    /**
     * @brief state Indicates the current state of the thread
     * @return The current state
     */
    [[nodiscard]] auto state() -> State;

    /**
     * @brief name The name of the thread
     * @return the name
     */
    [[nodiscard]] auto name() -> std::string;

    /**
     * @brief state_string The current state of the thread
     * @return The current state of the thread but represented as a string
     */
    [[nodiscard]] auto state_string() -> std::string;

    /**
     * @brief start Starts the thread asynchronuosly
     */
    void start();

    /**
     * @brief start_synchronuos Starts the thread synchronuosly
     */
    void start_synchronuos();


    [[nodiscard]] auto wait_for(State state, std::chrono::milliseconds timeout = std::chrono::seconds{5}) -> bool;

protected:
    /**
     * @brief run executed as the main loop
     */
    [[nodiscard]] auto run() -> int;

    [[nodiscard]] virtual auto custom_run() -> int;

    void exec();

    virtual void on_stop();

    /**
     * @brief finish Tells the main loop to finish and waits for the thread to exit
     */
    void finish();

    /**
     * @brief step executed each loop
     * @return false if the loop should stop immediatly, true otherwise
     */
    [[nodiscard]] virtual auto step() -> int;

    /**
     * @brief pre_run Executed before the thread loop starts
     * @return Thread loop will not start with a nonzero return value
     */
    [[nodiscard]] virtual auto pre_run() -> int;

    /**
     * @brief post_run Executed after the thread loop stops
     * @return The return value will be returned from the thread loop
     */
    [[nodiscard]] virtual auto post_run() -> int;

    std::condition_variable m_condition;
    bool m_quit { false };

private:
    void set_state(State state);

    bool m_use_custom_run { false };

    std::atomic<bool> m_run { true };

    std::atomic<int> m_exit_code { 0 };

    std::future<int> m_run_future {};

    std::string m_name {};

    State m_state { State::Initial };

    std::unique_ptr<std::thread> m_thread { nullptr };

    std::condition_variable m_state_condition;
};

}

#endif // THREADRUNNER_H
