#include <boost/test/unit_test.hpp>

#include "muonpi/analysis/dataseries.h"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(core_data_series_test, * utf::tolerance(0.00001))

BOOST_AUTO_TEST_CASE(empty_test)
{
    muonpi::data_series<double> series{10};

    BOOST_TEST(series.n() == 0);

    BOOST_TEST(series.mean(muonpi::data_series<double>::mean_t::arithmetic) == 0.0);
    BOOST_TEST(series.mean(muonpi::data_series<double>::mean_t::geometric) == 0.0);
    BOOST_TEST(series.mean(muonpi::data_series<double>::mean_t::harmonic) == 0.0);
    BOOST_TEST(series.median() == 0.0);
    BOOST_TEST(series.stddev() == 0.0);
    BOOST_TEST(series.variance() == 0.0);
    BOOST_TEST(series.rms() == 0.0);
    BOOST_TEST(series.current() == 0.0);
    BOOST_TEST(series.min() == 0.0);
    BOOST_TEST(series.max() == 0.0);
}

BOOST_AUTO_TEST_CASE(fill_test)
{
    muonpi::data_series<double> series{10};

    BOOST_TEST(series.n() == 0);
    series.add(1.0);
    BOOST_TEST(series.n() == 1);

    BOOST_TEST(series.mean(muonpi::data_series<double>::mean_t::arithmetic) == 1.0);
    BOOST_TEST(series.mean(muonpi::data_series<double>::mean_t::geometric) == 1.0);
    BOOST_TEST(series.mean(muonpi::data_series<double>::mean_t::harmonic) == 1.0);
    BOOST_TEST(series.median() == 1.0);
    BOOST_TEST(series.stddev() == 0.0);
    BOOST_TEST(series.variance() == 0.0);
    BOOST_TEST(series.rms() == 1.0);
    BOOST_TEST(series.current() == 1.0);
    BOOST_TEST(series.min() == 1.0);
    BOOST_TEST(series.max() == 1.0);

    series.add(1.0);
    series.add(1.0);
    series.add(1.0);

    series.add(1.5);
    series.add(0.5);
    series.add(1.5);
    series.add(0.5);
    series.add(1.5);
    series.add(0.5);

    BOOST_TEST(series.mean(muonpi::data_series<double>::mean_t::arithmetic) == 1.0);
    BOOST_TEST(series.mean(muonpi::data_series<double>::mean_t::geometric) == 0.955273);
    BOOST_TEST(series.mean(muonpi::data_series<double>::mean_t::harmonic) == 0.833333);
    BOOST_TEST(series.median() == 1.0);
    BOOST_TEST(series.stddev() == 0.387298);
    BOOST_TEST(series.variance() == 0.15);
    BOOST_TEST(series.rms() == 1.07238);
    BOOST_TEST(series.current() == 0.5);
    BOOST_TEST(series.min() == 0.5);
    BOOST_TEST(series.max() == 1.5);

}

BOOST_AUTO_TEST_SUITE_END()
