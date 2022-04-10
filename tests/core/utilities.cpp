#include "muonpi/utility.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(core_message_constructor_tests)

BOOST_AUTO_TEST_CASE(test_message_constructor) {
    muonpi::message_constructor constructor {' '};

    BOOST_TEST(constructor.get_string() == std::string {""});

    constructor.add_field("a");
    BOOST_TEST(constructor.get_string() == std::string {"a"});

    constructor.add_field("b");
    BOOST_TEST(constructor.get_string() == std::string {"a b"});

    constructor.add_field("c");
    BOOST_TEST(constructor.get_string() == std::string {"a b c"});

    constructor.add_field("hallo");
    BOOST_TEST(constructor.get_string() == std::string {"a b c hallo"});
}
BOOST_AUTO_TEST_SUITE_END()

struct fixture_message_parser {
    muonpi::message_parser parser {"this is a  test", ' '};
};

BOOST_FIXTURE_TEST_SUITE(core_message_parser_tests, fixture_message_parser)

BOOST_AUTO_TEST_CASE(test_message_parser_empty) {
    BOOST_TEST(!parser.empty());
}

BOOST_AUTO_TEST_CASE(test_message_parser_size) {
    BOOST_TEST(parser.size() == 4);
}

BOOST_AUTO_TEST_CASE(test_message_parser_get) {
    BOOST_TEST(parser.get() == std::string {"this is a  test"});
}

BOOST_AUTO_TEST_CASE(test_message_parser_at) {
    BOOST_TEST(parser[0] == "this");
    BOOST_TEST(parser[1] == "is");
    BOOST_TEST(parser[2] == "a");
    BOOST_TEST(parser[3] == "test");
    BOOST_TEST(parser[4] == std::string {});
}

BOOST_AUTO_TEST_SUITE_END()

struct fixture_message_parser_empty {
    muonpi::message_parser parser {"  ", ' '};
};

BOOST_FIXTURE_TEST_SUITE(core_message_parser_tests_empty, fixture_message_parser_empty)

BOOST_AUTO_TEST_CASE(test_message_parser_empty) {
    BOOST_TEST(parser.empty());
}

BOOST_AUTO_TEST_CASE(test_message_parser_size) {
    BOOST_TEST(parser.size() == 0);
}

BOOST_AUTO_TEST_CASE(test_message_parser_get) {
    BOOST_TEST(parser.get() == "  ");
}

BOOST_AUTO_TEST_CASE(test_message_parser_at) {
    BOOST_TEST(parser[0] == std::string {});
}

BOOST_AUTO_TEST_SUITE_END()

#include "muonpi/scopeguard.h"

BOOST_AUTO_TEST_SUITE(core_scope_guard_tests)
BOOST_AUTO_TEST_CASE(test_failure) {
    bool value {true};
    {
        muonpi::scope_guard guard {[&] { value = false; }};
    }

    BOOST_TEST(!value);
}
BOOST_AUTO_TEST_CASE(test_dismiss) {
    bool value {true};
    {
        muonpi::scope_guard guard {[&] { value = false; }};
        guard.dismiss();
    }

    BOOST_TEST(value);
}
BOOST_AUTO_TEST_SUITE_END()
