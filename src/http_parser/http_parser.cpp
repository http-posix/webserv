#include "http_parser.hpp"
#include <vector>

HttpParser::HttpParser(void)
{
	method_ = None;
	state_ = RequestLine;
	external_state_ = NeedMoreData;

}

HttpParser::HttpParser(const HttpParser& other)
{
	*this = other;
}

HttpParser& HttpParser::operator=(const HttpParser& other)
{
	if (this != &other)
{
		
	}
	return (*this);
}

HttpParser::~HttpParser(void)
{
}
