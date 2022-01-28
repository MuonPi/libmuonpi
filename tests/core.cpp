#define BOOST_TEST_MODULE muonpi core tests
#include <boost/test/included/unit_test.hpp>

namespace butf = boost::unit_test;

#include "muonpi/utility.h"


BOOST_AUTO_TEST_SUITE( core_utility_tests )

BOOST_AUTO_TEST_CASE( test_constructor )
{
    muonpi::message_constructor constructor {' '};
    BOOST_TEST(constructor.get_string() == std::string{""});

    constructor.add_field("a");
    BOOST_TEST(constructor.get_string() == std::string{"a"});

    constructor.add_field("b");
    BOOST_TEST(constructor.get_string() == std::string{"a b"});

    constructor.add_field("c");
    BOOST_TEST(constructor.get_string() == std::string{"a b c"});

    constructor.add_field("hallo");
    BOOST_TEST(constructor.get_string() == std::string{"a b c hallo"});
}

BOOST_AUTO_TEST_CASE( test_parser_1 )
{
    muonpi::message_parser parser{"", ' '};
    BOOST_TEST(parser.size() == 0);
    BOOST_TEST(parser.empty());
    BOOST_TEST(parser.get() == std::string{""});
}

BOOST_AUTO_TEST_SUITE_END()
