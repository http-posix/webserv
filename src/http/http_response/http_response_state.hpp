#ifndef HTTP_RESPONSE_STATE_H
#define HTTP_RESPONSE_STATE_H

//#include "config-mock.hpp"
# include <cstddef>
# include <cstdint>
# include <filesystem>
# include <fstream>
# include <iostream>
# include <map>
# include <map>
# include <string>
# include <string>
# include <vector>
#include "../src/http_request/http_request.hpp"

// struct	HttpRequest
// {
// 	std::string method_ = "GET";
// 	// Path relative to the project root used by tests
// 	std::string path_ = "test/response_test/mock_site/simple.html";
// 	std::string version_ = "HTTP/1.1";
// 	std::map<std::string, std::string> headers_ = {{"Host", "localhost:8080"},
// 		{"Accept",
// 		"text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"},
// 		{"Connection", "close"}, {"User-Agent", "test-client/1.0"}};
// 	std::string body_;
// };

enum	StatusCode
{
	OK = 200,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	NOT_ALLOWED = 405,
	OTHER = -1,
};

class HttpResponseState
{
private:
	const std::string httpVersion_ = "HTTP/1.1";
	StatusCode statusCode_;
	std::vector<std::pair<std::string, std::string>> headers_;
	std::vector<unsigned char> body_;

public:
	HttpResponseState();
	// HttpResponseState(const HttpResponseState &src);
	// HttpResponseState &operator=(const HttpResponseState &src);
	~HttpResponseState();

	int fill_response(const HttpRequest &request);
	void set_body(const HttpRequest &request);
	void set_statusCode(const HttpRequest &request);
	StatusCode get_statusCode();
	void set_headers(const HttpRequest &request);
	void add_header(const std::string &name, const std::string &value);
	std::string serialize(void);
	// //temp debug
	// void read_headers();
};

#endif
