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

HttpParserState HttpParser::Feed(char *str)
{
	input_buffer_ += str;
	int	loop = 0;

	while (loop == 0)
	{
		switch (state_)
		{
			case Error :
				// Feed function shouldn't be called if an error has been identified. This is a failsafe
				loop = -1;
			case RequestLine :
				loop = ParseRequestLine();
			case Header :
				loop = ParseHeaders();
			case Body :
				loop = ParseBody();
		}
	}
	return (external_state_);
}

int HttpParser::ParseMethod(std::string str)
{
	if (str == "GET")
		method_ = Get;
	else if (str == "DELETE")
		method_ = Delete;
	else if (str == "POST")
		method_ = Post;
	else
		return (-1);
	return (0);
}
