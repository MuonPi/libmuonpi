#include <boost/test/unit_test.hpp>

#include "muonpi/analysis/dataseries.h"

#include <numeric>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(core_data_series_test_small_sample, * utf::tolerance(0.00001))

BOOST_AUTO_TEST_CASE(empty_test)
{
    using data_s = muonpi::data_series<double>;

    data_s series{10};

    BOOST_TEST(series.n() == 0);

    BOOST_TEST(series.mean(data_s::mean_t::arithmetic) == 0.0);
    BOOST_TEST(series.mean(data_s::mean_t::geometric) == 0.0);
    BOOST_TEST(series.mean(data_s::mean_t::harmonic) == 0.0);
    BOOST_TEST(series.mean(data_s::mean_t::quadratic) == 0.0);
    BOOST_TEST(series.median() == 0.0);
    BOOST_TEST(series.stddev() == 0.0);
    BOOST_TEST(series.variance() == 0.0);
    BOOST_TEST(series.current() == 0.0);
    BOOST_TEST(series.min() == 0.0);
    BOOST_TEST(series.max() == 0.0);
    BOOST_TEST(series.sum() == 0.0);
}

BOOST_AUTO_TEST_CASE(fill_test)
{
    using data_s = muonpi::data_series<double>;

    data_s series{10};

    BOOST_TEST(series.n() == 0);
    series.add(1.0);
    BOOST_TEST(series.n() == 1);

    BOOST_TEST(series.mean(data_s::mean_t::arithmetic) == 1.0);
    BOOST_TEST(series.mean(data_s::mean_t::geometric) == 1.0);
    BOOST_TEST(series.mean(data_s::mean_t::harmonic) == 1.0);
    BOOST_TEST(series.mean(data_s::mean_t::quadratic) == 1.0);
    BOOST_TEST(series.median() == 1.0);
    BOOST_TEST(series.stddev() == std::numeric_limits<double>::infinity());
    BOOST_TEST(series.variance() == std::numeric_limits<double>::infinity());
    BOOST_TEST(series.current() == 1.0);
    BOOST_TEST(series.min() == 1.0);
    BOOST_TEST(series.max() == 1.0);
    BOOST_TEST(series.sum() == 1.0);

    series.add(1.0);
    series.add(1.0);
    series.add(1.0);

    series.add(1.5);
    series.add(0.5);
    series.add(1.5);
    series.add(0.5);
    series.add(1.5);
    series.add(0.5);

    BOOST_TEST(series.mean(data_s::mean_t::arithmetic) == 1.0);
    BOOST_TEST(series.mean(data_s::mean_t::geometric) == 0.917314);
    BOOST_TEST(series.mean(data_s::mean_t::harmonic) == 0.833333);
    BOOST_TEST(series.mean(data_s::mean_t::quadratic) == 1.07238);
    BOOST_TEST(series.median() == 1.0);
    BOOST_TEST(series.stddev() == 0.387298);
    BOOST_TEST(series.variance() == 0.15);
    BOOST_TEST(series.current() == 0.5);
    BOOST_TEST(series.min() == 0.5);
    BOOST_TEST(series.max() == 1.5);
    BOOST_TEST(series.sum() == 10.0);

}

BOOST_AUTO_TEST_SUITE_END()
