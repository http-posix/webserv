#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../src/http_parser/http_parser.hpp"

TEST_CASE("Incomplete request requiring more data") {
	HttpParser	p;

	CHECK(p.Feed("GET") == NeedMoreData);
}

TEST_CASE("InvalidMethod") {
	HttpParser p;

	p.Feed("GETT\r\n");
	CHECK(p.GetInternalState() == Error);
	CHECK(p.GetExternalState() == InvalidRequest);
}

TEST_CASE("Valid Simple GET Request") {
	HttpParser p;

	p.Feed("GET / HTTP/1.1\r\n");
	p.Feed("Connection: Close\r\n");
	p.Feed("Host: google.com\r\n");
	p.Feed("Accept: */*\r\n");
	p.Feed("User-agent: iamroot.tech\r\n");
	p.Feed("\r\n");

	CHECK(p.GetMethod() == Get);
	CHECK(p.GetExternalState() == Complete);
	CHECK(p.GetInternalState() == Done);
	CHECK(p.GetBodyExpectedLength() == 0);
	CHECK(p.GetVersion() == "HTTP/1.1");
}

