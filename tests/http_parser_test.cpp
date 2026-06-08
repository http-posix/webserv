#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../src/http_parser/http_parser.hpp"

TEST_CASE("Incomplete request requiring more data") {
	HttpParser	p;

	CHECK(p.Feed("GET") == NeedMoreData);
}
