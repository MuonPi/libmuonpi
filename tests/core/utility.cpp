#include <boost/test/unit_test.hpp>

#include "muonpi/utility.h"


BOOST_AUTO_TEST_SUITE( core_message_constructor_tests )

BOOST_AUTO_TEST_CASE( test_message_constructor )
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
BOOST_AUTO_TEST_SUITE_END()


struct fixture_message_parser {
    fixture_message_parser() = default;
    ~fixture_message_parser() = default;

    muonpi::message_parser parser {"this is a test", ' '};
};

BOOST_FIXTURE_TEST_SUITE( core_message_parser_tests, fixture_message_parser )

    BOOST_AUTO_TEST_CASE( test_message_parser_empty )
    {
        BOOST_TEST(!parser.empty());
    }

    BOOST_AUTO_TEST_CASE( test_message_parser_size )
    {
        BOOST_TEST(parser.size() == 4);
    }

    BOOST_AUTO_TEST_CASE( test_message_parser_incomplete )
    {
        BOOST_FAIL("Test is incomplete.");
    }

BOOST_AUTO_TEST_SUITE_END()

