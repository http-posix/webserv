#include "http_parser.hpp"
#include <iostream>

/*
 * GET /index.html HTTP/1.1\r\n
 * Host: example.com\r\n
 * User-Agent: CustomClient/1.0\r\n
 * Accept: \r\n
 * \r\n
*/
/*
 * POST /api/login HTTP/1.1\r\n
 * Host: api.example.com\r\n
 * Content-Type: application/json\r\n
 * Content-Length: 42\r\n
 * \r\n
 * {"username":"alice","password":"secret"}
*/

int	main(void)
{
	HttpParser	test;

	test.Feed("GET /index.html HTTP/1.0\r\nHost: example.com\r\nContent-Length: 5\r\n\r\nhe");
	test.PrintParserRequest();
	// Need more data (2/5 bytes of body)
	test.Feed("llo");
	test.PrintParserRequest();
	return (0);
}
