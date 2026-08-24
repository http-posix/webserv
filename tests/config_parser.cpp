#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

//#include "../src/config_parser/config_parser.hpp"
#include "../src/config/parser/parser.hpp"

//#include "../src/app_exception/app_exception.hpp"
#include "../src/utils/app_exception/app_exception.hpp"

// Helper: parse a file, expecting it to be rejected with a ConfigException.
static void expectParseFailure(const std::string& file)
{
	ConfigParser p;
	REQUIRE(p.readFile(file) == 0);
	CHECK_THROWS_AS(p.parseFromString(), ConfigException);
}

// Helper: parse a valid file and return the resulting Config.
static Config parseFile(const std::string& file)
{
	ConfigParser p;
	REQUIRE(p.readFile(file) == 0);
	p.parseFromString();
	return p.getConfig();
}

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
	CHECK(c.servers[0].error_pages.at(505) == "error_pages/505.html");
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

// ---------------------------------------------------------------------------
// Valid configuration files
// ---------------------------------------------------------------------------

TEST_CASE("Valid: server with every supported directive")
{
	Config c = parseFile("tests/config_test_files/valid_full_server");

	CHECK(c.servers.size() == 1);
	CHECK(c.servers[0].hostname == "127.0.0.1");
	CHECK(c.servers[0].listen_port.size() == 1);
	CHECK(c.servers[0].listen_port[0] == 8002);
	CHECK(c.servers[0].root == "docs/fusion_web/");
	CHECK(c.servers[0].index == "index.html");
	CHECK(c.servers[0].client_max_body_size == 3000);
	CHECK(c.servers[0].error_pages.at(404) == "error_pages/404.html");
	CHECK(c.servers[0].error_pages.at(505) == "error_pages/505.html");

	CHECK(c.servers[0].locations.size() == 1);
	const LocationConfig& loc = c.servers[0].locations[0];
	CHECK(loc.uri_path == "/cgi-bin");
	CHECK(loc.allowed_methods.size() == 3);
	CHECK(loc.allowed_methods[0] == "GET");
	CHECK(loc.allowed_methods[1] == "POST");
	CHECK(loc.allowed_methods[2] == "DELETE");
	CHECK(loc.autoindex == false);
	CHECK(loc.index == "time.py");
	CHECK(loc.root == "./cgi/");
	CHECK(loc.upload_enable == true);
	CHECK(loc.upload_location == "./uploads");
	CHECK(loc.redirections.first == 302);
	CHECK(loc.redirections.second == "/new_place");
}

TEST_CASE("Valid: server with multiple locations")
{
	Config c = parseFile("tests/config_test_files/valid_multiple_locations");

	CHECK(c.servers.size() == 1);
	CHECK(c.servers[0].locations.size() == 3);

	const LocationConfig& root = c.servers[0].locations[0];
	CHECK(root.uri_path == "/");
	CHECK(root.allowed_methods.size() == 1);
	CHECK(root.allowed_methods[0] == "GET");
	CHECK(root.autoindex == true);
	CHECK(root.index == "index.html");

	const LocationConfig& upload = c.servers[0].locations[1];
	CHECK(upload.uri_path == "/upload");
	CHECK(upload.allowed_methods.size() == 3);
	CHECK(upload.upload_enable == true);
	CHECK(upload.upload_location == "www/uploads");
	CHECK(upload.root == "www");
	CHECK(upload.index == "upload.html");

	const LocationConfig& redir = c.servers[0].locations[2];
	CHECK(redir.uri_path == "/old-page");
	CHECK(redir.redirections.first == 301);
	CHECK(redir.redirections.second == "/new-page");
}

TEST_CASE("Valid: multiple servers with multiple ports")
{
	Config c = parseFile("tests/config_test_files/valid_multiple_servers");

	CHECK(c.servers.size() == 3);
	CHECK(c.servers[0].hostname == "localhost");
	CHECK(c.servers[0].listen_port.size() == 2);
	CHECK(c.servers[0].listen_port[0] == 8080);
	CHECK(c.servers[0].listen_port[1] == 8081);
	CHECK(c.servers[0].error_pages.at(404) == "error_pages/404.html");

	CHECK(c.servers[1].hostname == "mfw.com");
	CHECK(c.servers[1].listen_port[0] == 9090);
	CHECK(c.servers[1].client_max_body_size == 100000);

	CHECK(c.servers[2].hostname == "third.com");
	CHECK(c.servers[2].listen_port[0] == 1212);
}

TEST_CASE("Valid: comments are stripped")
{
	Config c = parseFile("tests/config_test_files/valid_comments");

	CHECK(c.servers.size() == 1);
	CHECK(c.servers[0].hostname == "localhost");
	CHECK(c.servers[0].listen_port[0] == 8080);
	CHECK(c.servers[0].client_max_body_size == 15 * 1024);
	CHECK(c.servers[0].locations.size() == 1);
	CHECK(c.servers[0].locations[0].allowed_methods.size() == 2);
	CHECK(c.servers[0].locations[0].autoindex == true);
}

TEST_CASE("Valid: client_max_body_size size units")
{
	Config c = parseFile("tests/config_test_files/valid_units");

	CHECK(c.servers.size() == 6);
	CHECK(c.servers[0].client_max_body_size == 15 * 1024);
	CHECK(c.servers[1].client_max_body_size == 2 * 1024 * 1024);
	CHECK(c.servers[2].client_max_body_size == 1 * 1024 * 1024 * 1024);
	CHECK(c.servers[3].client_max_body_size == 100 * 1024);
	CHECK(c.servers[4].client_max_body_size == 1 * 1024 * 1024);
	CHECK(c.servers[5].client_max_body_size == 1024);
}

TEST_CASE("Valid: quoted values")
{
	Config c = parseFile("tests/config_test_files/valid_quoted");

	CHECK(c.servers[0].hostname == "foo.bar");
	CHECK(c.servers[0].index == "index.html");
	CHECK(c.servers[0].locations[0].root == "/tmp root");
	CHECK(c.servers[0].locations[0].index == "my index.html");
}

TEST_CASE("Valid: methods separated by commas")
{
	Config c = parseFile("tests/config_test_files/valid_methods_commas");

	CHECK(c.servers[0].locations[0].allowed_methods.size() == 3);
	CHECK(c.servers[0].locations[0].allowed_methods[0] == "GET");
	CHECK(c.servers[0].locations[0].allowed_methods[1] == "POST");
	CHECK(c.servers[0].locations[0].allowed_methods[2] == "DELETE");
	CHECK(c.servers[0].locations[1].allowed_methods.size() == 3);
}

TEST_CASE("Valid: CRLF line endings")
{
	Config c = parseFile("tests/config_test_files/valid_crlf");

	CHECK(c.servers[0].hostname == "127.0.0.1");
	CHECK(c.servers[0].listen_port[0] == 80);
	CHECK(c.servers[0].locations.size() == 1);
	CHECK(c.servers[0].locations[0].uri_path == "/");
	CHECK(c.servers[0].locations[0].autoindex == false);
}

TEST_CASE("Valid: redirection directives")
{
	Config c = parseFile("tests/config_test_files/valid_redirection");

	CHECK(c.servers[0].locations.size() == 3);
	CHECK(c.servers[0].locations[0].redirections.first == 301);
	CHECK(c.servers[0].locations[0].redirections.second == "/new");
	CHECK(c.servers[0].locations[1].redirections.first == 308);
	CHECK(c.servers[0].locations[1].redirections.second == "http://www.example.com/");
	CHECK(c.servers[0].locations[2].redirections.first == 302);
	CHECK(c.servers[0].locations[2].redirections.second == "https://example.org/a b");
}

TEST_CASE("Valid: error_page overwrites a previous entry for the same code")
{
	Config c = parseFile("tests/config_test_files/valid_error_pages_overwrite");

	CHECK(c.servers[0].error_pages.at(404) == "error_pages/other404.html");
	CHECK(c.servers[0].error_pages.at(500) == "error_pages/500.html");
}

TEST_CASE("Valid: autoindex and upload_enable toggles")
{
	Config c = parseFile("tests/config_test_files/valid_autoindex_upload");

	CHECK(c.servers[0].locations[0].autoindex == true);
	CHECK(c.servers[0].locations[0].upload_enable == true);
	CHECK(c.servers[0].locations[0].upload_location == "/data/uploads");
	CHECK(c.servers[0].locations[1].autoindex == false);
	CHECK(c.servers[0].locations[1].upload_enable == false);
}

TEST_CASE("Valid: whole server on a single line")
{
	Config c = parseFile("tests/config_test_files/valid_single_line_server");

	CHECK(c.servers.size() == 1);
	CHECK(c.servers[0].hostname == "localhost");
	CHECK(c.servers[0].listen_port[0] == 80);
}

TEST_CASE("Valid: server containing only a location")
{
	Config c = parseFile("tests/config_test_files/valid_location_only");

	CHECK(c.servers.size() == 1);
	CHECK(c.servers[0].locations.size() == 1);
	CHECK(c.servers[0].locations[0].uri_path == "/");
	CHECK(c.servers[0].locations[0].allowed_methods.size() == 1);
	CHECK(c.servers[0].locations[0].allowed_methods[0] == "GET");
}

TEST_CASE("Valid: nginx-style configuration")
{
	Config c = parseFile("tests/config_test_files/valid_nginx_style");

	CHECK(c.servers.size() == 1);
	CHECK(c.servers[0].hostname == "localhost");
	CHECK(c.servers[0].listen_port[0] == 80);
	CHECK(c.servers[0].root == "/var/www/");
	CHECK(c.servers[0].index == "index.html");
	CHECK(c.servers[0].client_max_body_size == 1 * 1024 * 1024);
	CHECK(c.servers[0].error_pages.at(404) == "/errors/404.html");
	CHECK(c.servers[0].error_pages.at(500) == "/errors/500.html");
	CHECK(c.servers[0].error_pages.at(502) == "/errors/500.html");

	CHECK(c.servers[0].locations.size() == 3);
	const LocationConfig& root = c.servers[0].locations[0];
	CHECK(root.uri_path == "/");
	CHECK(root.allowed_methods.size() == 1);
	CHECK(root.allowed_methods[0] == "GET");
	CHECK(root.autoindex == true);
	CHECK(root.index == "index.html");
	CHECK(root.root == "/var/www/");

	const LocationConfig& upload = c.servers[0].locations[1];
	CHECK(upload.uri_path == "/upload");
	CHECK(upload.allowed_methods.size() == 3);
	CHECK(upload.upload_enable == true);
	CHECK(upload.upload_location == "/var/www/uploads");
	CHECK(upload.autoindex == false);

	const LocationConfig& redir = c.servers[0].locations[2];
	CHECK(redir.redirections.first == 301);
	CHECK(redir.redirections.second == "/new-page");
}

// ---------------------------------------------------------------------------
// Invalid configuration files (must be rejected with a ConfigException)
// ---------------------------------------------------------------------------

TEST_CASE("Invalid configs: block syntax")
{
	SUBCASE("missing semicolon")
	{
		expectParseFailure("tests/config_test_files/invalid_missing_semicolon");
	}
	SUBCASE("unterminated server block")
	{
		expectParseFailure("tests/config_test_files/invalid_unterminated_server");
	}
	SUBCASE("unterminated location block")
	{
		expectParseFailure("tests/config_test_files/invalid_unterminated_location");
	}
	SUBCASE("extra closing brace")
	{
		expectParseFailure("tests/config_test_files/invalid_extra_close_brace");
	}
	SUBCASE("duplicate 'server' keyword")
	{
		expectParseFailure("tests/config_test_files/invalid_duplicate_server_keyword");
	}
	SUBCASE("trailing semicolon after server block")
	{
		expectParseFailure("tests/config_test_files/invalid_trailing_semicolon");
	}
	SUBCASE("location without URI")
	{
		expectParseFailure("tests/config_test_files/invalid_location_no_uri");
	}
	SUBCASE("location nested inside location")
	{
		expectParseFailure("tests/config_test_files/invalid_nested_location");
	}
	SUBCASE("content outside a server block")
	{
		expectParseFailure("tests/config_test_files/invalid_content_outside_server");
	}
	SUBCASE("no server block at all")
	{
		expectParseFailure("tests/config_test_files/invalid_no_server_block");
	}
}

TEST_CASE("Invalid configs: 'listen' directive")
{
	SUBCASE("port out of range (65536)")
	{
		expectParseFailure("tests/config_test_files/invalid_port_out_of_range");
	}
	SUBCASE("port too large to fit in a number")
	{
		expectParseFailure("tests/config_test_files/invalid_port_huge");
	}
	SUBCASE("non numeric port")
	{
		expectParseFailure("tests/config_test_files/invalid_port_non_numeric");
	}
	SUBCASE("negative port")
	{
		expectParseFailure("tests/config_test_files/invalid_port_negative");
	}
	SUBCASE("float port")
	{
		expectParseFailure("tests/config_test_files/invalid_port_float");
	}
	SUBCASE("two ports on one line")
	{
		expectParseFailure("tests/config_test_files/invalid_port_two_values");
	}
	SUBCASE("hostname:port form is not supported")
	{
		expectParseFailure("tests/config_test_files/invalid_port_hostname");
	}
}

TEST_CASE("Invalid configs: empty values for string directives")
{
	SUBCASE("empty hostname")
	{
		expectParseFailure("tests/config_test_files/invalid_empty_hostname");
	}
	SUBCASE("empty root")
	{
		expectParseFailure("tests/config_test_files/invalid_empty_root");
	}
	SUBCASE("empty index")
	{
		expectParseFailure("tests/config_test_files/invalid_empty_index");
	}
	SUBCASE("empty methods")
	{
		expectParseFailure("tests/config_test_files/invalid_empty_methods");
	}
	SUBCASE("empty upload_store")
	{
		expectParseFailure("tests/config_test_files/invalid_empty_upload_store");
	}
}

TEST_CASE("Invalid configs: 'error_page' directive")
{
	SUBCASE("missing path")
	{
		expectParseFailure("tests/config_test_files/invalid_error_page_no_path");
	}
	SUBCASE("non numeric status code")
	{
		expectParseFailure("tests/config_test_files/invalid_error_page_non_numeric");
	}
	SUBCASE("negative status code")
	{
		expectParseFailure("tests/config_test_files/invalid_error_page_negative");
	}
	SUBCASE("first directive invalid, second valid")
	{
		expectParseFailure("tests/config_test_files/invalid_error_page_bad_then_good");
	}
}

TEST_CASE("Invalid configs: 'client_max_body_size' directive")
{
	SUBCASE("missing number")
	{
		expectParseFailure("tests/config_test_files/invalid_body_size_no_number");
	}
	SUBCASE("unknown unit")
	{
		expectParseFailure("tests/config_test_files/invalid_body_size_bad_unit");
	}
	SUBCASE("float value")
	{
		expectParseFailure("tests/config_test_files/invalid_body_size_float");
	}
	SUBCASE("missing semicolon")
	{
		expectParseFailure("tests/config_test_files/invalid_body_size_no_semicolon");
	}
}

TEST_CASE("Invalid configs: location directives")
{
	SUBCASE("autoindex with a non on/off value")
	{
		expectParseFailure("tests/config_test_files/invalid_autoindex_bad_value");
	}
	SUBCASE("upload_enable with a non on/off value")
	{
		expectParseFailure("tests/config_test_files/invalid_upload_enable_bad_value");
	}
	SUBCASE("return without URL")
	{
		expectParseFailure("tests/config_test_files/invalid_return_no_url");
	}
	SUBCASE("return without status code")
	{
		expectParseFailure("tests/config_test_files/invalid_return_no_code");
	}
	SUBCASE("unknown directive in location")
	{
		expectParseFailure("tests/config_test_files/invalid_unknown_directive_location");
	}
}

TEST_CASE("Invalid configs: unknown directives and missing server")
{
	SUBCASE("unknown directive in server block")
	{
		expectParseFailure("tests/config_test_files/invalid_unknown_directive_server");
	}
	SUBCASE("autoindex is not a server level directive")
	{
		expectParseFailure("tests/config_test_files/invalid_server_level_autoindex");
	}
}

TEST_CASE("Invalid configs: file contains no server block")
{
	SUBCASE("only comments")
	{
		expectParseFailure("tests/config_test_files/invalid_comment_only");
	}
	SUBCASE("only whitespace")
	{
		expectParseFailure("tests/config_test_files/invalid_whitespace_only");
	}
}

// ---------------------------------------------------------------------------
// Known bugs in the configuration parser (tests intentionally fail)
//
// These test cases assert the *desired* behaviour. They currently fail,
// documenting parser bugs that have not been fixed yet. Once the parser is
// fixed, these test cases should turn green.
// ---------------------------------------------------------------------------

TEST_CASE("Known bugs: invalid values are accepted")
{
	SUBCASE("error_page status code out of range should be rejected")
	{
		expectParseFailure("tests/config_test_files/bug_error_page_code_out_of_range");
	}
	SUBCASE("return status code out of range should be rejected")
	{
		expectParseFailure("tests/config_test_files/bug_return_code_out_of_range");
	}
	SUBCASE("listen port 0 should be rejected")
	{
		expectParseFailure("tests/config_test_files/bug_listen_port_zero");
	}
	SUBCASE("server block without any directive should be rejected")
	{
		expectParseFailure("tests/config_test_files/bug_empty_server_block");
	}
	SUBCASE("non-HTTP methods should be rejected")
	{
		expectParseFailure("tests/config_test_files/bug_invalid_methods_accepted");
	}
}

/*
 * TODO: These tests could be considered as bugs.
 * However currently, I think these are improvements that could be made
 * but not necessarily incorrect usage.

 TEST_CASE("Known bugs: values are not handled correctly")
{
	SUBCASE("duplicate methods should be de-duplicated")
	{
		Config c = parseFile("tests/config_test_files/bug_duplicate_methods");
		CHECK(c.servers[0].locations[0].allowed_methods.size() == 2);
	}
	SUBCASE("error_page with multiple codes on same line should throw an error")
	{
		expectParseFailure("tests/config_test_files/bug_error_page_multiple_codes");
	}
	SUBCASE("index with multiple values should keep them separated")
	{
		Config c = parseFile("tests/config_test_files/bug_index_multiple_values");
		CHECK(c.servers[0].index == "index.html index.php");
	}
	SUBCASE("a '#' inside a quoted value should not be treated as a comment")
	{
		Config c = parseFile("tests/config_test_files/bug_comment_inside_quoted_string");
		CHECK(c.servers[0].index == "foo#bar");
	}
}
*/
