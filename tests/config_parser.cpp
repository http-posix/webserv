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

// The next three test cases don't make a destinition between the
// different errors. Currently the main point I consider is that we
// return an error. Not necessarily informing the user what the error is.
TEST_CASE("Open empty file")
{
	ConfigParser p;
	std::string file = "tests/config_test_files/emptyfile";

	CHECK(p.readFile(file) != 0);
}

TEST_CASE("Open non existing file")
{
	ConfigParser p;
	std::string file = "tests/config_test_files/nonexistingfile";

	CHECK(p.readFile(file) != 0);
}

TEST_CASE("Open directory instead of file")
{
	ConfigParser p;
	std::string file = "tests/";

	CHECK(p.readFile(file) != 0);
}
