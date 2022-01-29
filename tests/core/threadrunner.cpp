#include <boost/test/unit_test.hpp>

#include "muonpi/threadrunner.h"
#include "muonpi/log.h"

#include <thread>
#include <chrono>
#include <exception>

namespace utf = boost::unit_test;

struct runner_failure : public muonpi::thread_runner {
public:
    runner_failure(std::string name)
        : muonpi::thread_runner{std::move(name), true}
    {
    }
protected:
    [[nodiscard]] auto custom_run() -> int override
    {
        throw std::runtime_error{"This is an intentional error."};
    }
};

struct fixture_thread_runner {
    void setup()
    {
        muonpi::log::system::setup(muonpi::log::Level::Info, [](int c){exit(c);}, std::cerr);
    }
    inline static muonpi::thread_runner runner{"name"};
};

BOOST_TEST_GLOBAL_FIXTURE(fixture_thread_runner);

BOOST_AUTO_TEST_SUITE(core_thread_runner_nominal_test)

BOOST_AUTO_TEST_CASE(test_initial_state)
{
    BOOST_TEST(fixture_thread_runner::runner.name() == "name");

    BOOST_TEST((fixture_thread_runner::runner.state() == muonpi::thread_runner::State::Initial));
}
BOOST_AUTO_TEST_CASE(test_startup)
{
    fixture_thread_runner::runner.start();
    BOOST_TEST(fixture_thread_runner::runner.wait_for(muonpi::thread_runner::State::Initialising, std::chrono::milliseconds{10}));
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    BOOST_TEST(fixture_thread_runner::runner.wait_for(muonpi::thread_runner::State::Running, std::chrono::seconds{1}));
}
BOOST_AUTO_TEST_CASE(test_shutdown)
{
    fixture_thread_runner::runner.stop(0);
    BOOST_TEST(fixture_thread_runner::runner.wait_for(muonpi::thread_runner::State::Finalising, std::chrono::milliseconds{10}));
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    BOOST_TEST(fixture_thread_runner::runner.wait_for(muonpi::thread_runner::State::Stopped, std::chrono::seconds{1}));
    BOOST_TEST(fixture_thread_runner::runner.wait() == 0);
}
BOOST_AUTO_TEST_SUITE_END()

