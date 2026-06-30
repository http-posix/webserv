#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "config_parser/config_parser.hpp"
#include "../src/config_parser/config_parser.hpp"

TEST_CASE("Open and print content config file")
{
	ConfigParser p;
	std::string file = "tests/config_test_files/simpletest";

	REQUIRE(p.readFile(file) == 0);
}
TEST_CASE("Open file with insufficient permissions")
{
	ConfigParser p;
	std::string file = "tests/config_test_files/chmod000";

	CHECK(p.readFile(file) != 0);
}
