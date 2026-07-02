#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../src/http_parser/http_parser.hpp"
#include "../src/http_request/http_request.hpp"
#include "../src/http_response/http_response_state.hpp"

//This file creates multiple test_cases to check correct functioning of a request life cycle beginning from the parser and ending in the response.

/*
 This is the string that the response generator should send through the epoll at the very end of requests lifecycle:
"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 12\r\nConnection: close\r\n\r\nHello world!"
*/
TEST_CASE("Simple lifecycle of get request")
{
	HttpParser p;

	p.Feed("GET tests/mock_files/simple.html HTTP/1.1\r\n");
	p.Feed("Connection: Close\r\n");
	p.Feed("Host: google.com\r\n");
	p.Feed("Accept: */*\r\n");
	p.Feed("User-agent: iamroot.tech\r\n");
	p.Feed("\r\n");

	HttpRequest req;

	p.FillRequest(&req);
	
	HttpResponseState resp;

	resp.fill_response(req);
	CHECK(resp.get_statusCode() == OK);
}
