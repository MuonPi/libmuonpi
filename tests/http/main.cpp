#define BOOST_TEST_MODULE muonpi http tests
#include <boost/test/included/unit_test.hpp>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_CASE(http_dummy_test, * utf::expected_failures(1))
{
    BOOST_TEST(false); // dummy test so it isn't empty until real tests are written
}
