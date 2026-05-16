#include "http_parser.hpp"
#include <iostream>
#include <vector>

HttpParser::HttpParser(void)
{
	method_ = None;
	state_ = RequestLine;
	external_state_ = NeedMoreData;
	std::cout << "Parser initiated" << std::endl;
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
	std::cout << "Parser Destroyed" << std::endl;
}

HttpParserState HttpParser::Feed(const char *str)
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
				break ;
			case RequestLine :
				loop = ParseRequestLine();
				break ;
			case Header :
				loop = ParseHeaders();
				break ;
			case Body :
				loop = ParseBody();
				break ;
		}
	}
	return (external_state_);
}

int HttpParser::ParseHeaders()
{
	std::cout << "Parse headers called" << std::endl;
	return (-1);
}

int HttpParser::ParseBody()
{
	std::cout << "Parse  body called" << std::endl;
	return (-1);
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

int HttpParser::ParseVersion(std::string str)
{
	if (str == "HTTP/1.0")
		version_ = "HTTP/1.0";
	else if (str == "HTTP/1.1")
		version_ = "HTTP/1.1";
	else
		return (-1);
	return (0);
}

int HttpParser::ParseRequestLine()
{
	input_i_ = input_buffer_.find("\r\n");
		if (input_i_ == std::string::npos)
			return (-1);

	size_t i;
	i = input_buffer_.find(" ", 0);
	if (i == std::string::npos)
	{
		state_ = Error;
		external_state_ = InvalidRequest;
		return (-1);
	}
	
	std::string temp;
	temp = input_buffer_.substr(0, i);
	if (ParseMethod(temp) == -1)
		return (-1);
		// Invalid Method
	size_t j;
	j = input_buffer_.find(" ", i);
	if (j == std::string::npos)
	{
		state_ = Error;
		external_state_ = InvalidRequest;
		return (-1);
	}
	// Hard copy the path since we don't need to check anything
	version_ = input_buffer_.substr(i, j);
	temp = input_buffer_.substr(j, input_i_);
	if (ParseVersion(temp) == -1)
		return (-1);	// TODO
						// Invalid Version.
	
	// Trim beginning of buffer to remove the request line. +2 to remove \r\n char as well.
	input_buffer_ = input_buffer_.substr(input_i_ + 2);
	state_ = Header;
	return (0);	
}
void HttpParser::PrintParserRequest()
{
	std::cout << "PRINTING PARSER INFO::\n\n";
	std::cout << "Method: " << method_ << "\n";
	std::cout << "Path: " << path_ << "\n";
	std::cout << "Version: " << version_ << "\n";
	std::cout << "Input Buffer: " << input_buffer_;
	std::cout << std::endl;
}
