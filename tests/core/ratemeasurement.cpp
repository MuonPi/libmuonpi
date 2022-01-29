#include <boost/test/unit_test.hpp>
#include <boost/range/irange.hpp>

#include "muonpi/analysis/ratemeasurement.h"

#include <numeric>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(core_rate_measurement_test, * utf::tolerance(0.00001))

using rate_m = muonpi::rate_measurement<double>;

BOOST_AUTO_TEST_CASE(empty_test)
{
    rate_m rate{10, std::chrono::seconds{3}};

    BOOST_TEST(rate.current() == 0.0);
}

BOOST_AUTO_TEST_CASE(fill_test)
{
    rate_m rate{10, std::chrono::seconds{3}};
    auto now { std::chrono::system_clock::now() };

    BOOST_TEST(rate.current() == 0.0);

    for ([[maybe_unused]] auto i : boost::irange(0, 23)) {
        rate.increase_counter();
    }

    rate.step(now + std::chrono::seconds{3});
    BOOST_TEST(rate.current() == 7.6666);

    for ([[maybe_unused]] auto i : boost::irange(0, 15)) {
        rate.increase_counter();
    }

    rate.step(now + std::chrono::seconds{6});
    BOOST_TEST(rate.current() == 5);

    for ([[maybe_unused]] auto i : boost::irange(0, 30)) {
        rate.increase_counter();
    }

    rate.step(now + std::chrono::seconds{10});
    BOOST_TEST(rate.current() == 7.5);
}

BOOST_AUTO_TEST_SUITE_END()
