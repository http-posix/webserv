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

TEST_CASE("Valid Simple POST Request")
{
	HttpParser p;

	p.Feed("POST /somefilegoeshere HTTP/1.1");
	p.Feed("\r\n");
	p.Feed("Host: httpbin.org");
	p.Feed("\r\n");
	p.Feed("Content-Type: application/json");
	p.Feed("\r\n");
	p.Feed("Content-Length: 35");
	p.Feed("\r\n");
	p.Feed("\r\n");
	p.Feed("{\"request\":\"give me some response\"}");
	p.Feed("\r\n\r\n");

	CHECK(p.GetInternalState() == Done);
	CHECK(p.GetExternalState() == Complete);
	
	CHECK(p.GetMethod() == Post);
	CHECK(p.GetPath() == "/somefilegoeshere");
	CHECK(p.GetVersion() == "HTTP/1.1");
	CHECK(p.GetBodyExpectedLength() == 35);
	CHECK(p.GetBody() == "{\"request\":\"give me some response\"}");
}
TEST_CASE("Valid DELETE Request") {
	HttpParser p;

	p.Feed("DELETE /resource/123 HTTP/1.0\r\n");
	p.Feed("Host: example.com\r\n");
	p.Feed("Connection: Close\r\n");
	p.Feed("\r\n");

	CHECK(p.GetMethod() == Delete);
	CHECK(p.GetPath() == "/resource/123");
	CHECK(p.GetVersion() == "HTTP/1.0");
	CHECK(p.GetExternalState() == Complete);
	CHECK(p.GetInternalState() == Done);
	CHECK(p.GetBodyExpectedLength() == 0);
}

TEST_CASE("Multiple headers with same name (combined with comma)") {
	HttpParser p;

	p.Feed("GET / HTTP/1.1\r\n");
	p.Feed("Accept: text/html\r\n");
	p.Feed("Accept: application/json\r\n");
	p.Feed("Host: example.com\r\n");
	p.Feed("\r\n");

	auto headers = p.GetHeaders();
	CHECK(headers.find("accept") != headers.end());
	CHECK(headers["accept"] == "text/html,application/json");
	CHECK(p.GetExternalState() == Complete);
}

TEST_CASE("POST request with large Content-Length") {
	HttpParser p;

	std::string large_body(1000, 'a');
	
	p.Feed("POST /upload HTTP/1.1\r\n");
	p.Feed("Host: example.com\r\n");
	p.Feed("Content-Length: 1000\r\n");
	p.Feed("\r\n");
	p.Feed(large_body.c_str());

	CHECK(p.GetMethod() == Post);
	CHECK(p.GetBodyExpectedLength() == 1000);
	CHECK(p.GetBody() == large_body);
	CHECK(p.GetExternalState() == Complete);
}
TEST_CASE("Invalid header with leading whitespace") {
	HttpParser p;

	p.Feed("GET / HTTP/1.1\r\n");
	p.Feed(" Host: example.com\r\n");
	p.Feed("\r\n");
	
	// Leading whitespace in header should cause InvalidRequest
	CHECK(p.GetExternalState() == InvalidRequest);
	CHECK(p.GetInternalState() == Error);
}

