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

// Test cases that check numerous amounts of config files

TEST_CASE("Simple Config File with single Server Block")
{
	ConfigParser p;
	std::string file = "tests/config_test_files/single_server_block";

	//Check if file gets read correctly
	CHECK(p.readFile(file) == 0);
	p.parseFromString();	
	Config c = p.getConfig();

	CHECK(c.servers[0].hostname == "127.0.0.1");
	CHECK(c.servers[0].listen_port[0] == 8002);
	CHECK(c.servers[0].error_pages.at(404) == "error_pages/404.html");
	CHECK(c.servers[0].error_pages.at(303) == "error_pages/303.html");
	CHECK(c.servers[0].root == "docs/fusion_web/");
}

TEST_CASE("Config File with two simple servers")
{
	ConfigParser p;
	std::string file = "tests/config_test_files/two_simple_servers";

	CHECK(p.readFile(file) == 0);
	p.parseFromString();
	
	Config c = p.getConfig();

	CHECK(c.servers[0].hostname == "localhost");
	CHECK(c.servers[0].listen_port[0] == 8080);
	CHECK(c.servers[1].hostname == "mfw.com");
	CHECK(c.servers[1].listen_port[0] == 9090);
	CHECK(c.servers[1].listen_port[1] == 1212);
}

TEST_CASE("Testing location file")
{
	ConfigParser p;
	std::string file = "tests/config_test_files/simple_location";

	CHECK(p.readFile(file) == 0);
	p.parseFromString();
	
	Config c = p.getConfig();
	CHECK(c.servers[0].locations[0].uri_path == "/non_existing");
	CHECK(c.servers[0].locations[0].index == "/index_non_existing");
	CHECK(c.servers[0].locations[0].allowed_methods[0] == "GET");
	CHECK(c.servers[0].locations[0].allowed_methods[1] == "POST");
	CHECK(c.servers[0].locations[0].allowed_methods[2] == "DELETE");
}

//TODO
// No path given for location should crash
// invalid methods should not be stored. Instead of strings, have enums
// non existing index
// 
// listen_port too large
// redirection for locations does not need an integer for the pair?

