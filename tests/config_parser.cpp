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
