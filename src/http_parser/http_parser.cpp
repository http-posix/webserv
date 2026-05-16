#include "http_parser.hpp"
#include <iostream>
#include <vector>

static bool is_token_char(unsigned char c)
{
	// RFC 7230 token = 1*tchar; keep it simple and strict enough.
	// tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /
	//         "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA
	if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
		return true;
	switch (c)
	{
		case '!': case '#': case '$': case '%': case '&': case '\'': case '*':
		case '+': case '-': case '.': case '^': case '_': case '`': case '|':
		case '~':
			return true;
		default:
			return false;
	}
}

HttpParser::HttpParser(void)
{
	method_ = None;
	state_ = RequestLine;
	external_state_ = NeedMoreData;
	body_expected_len_ = 0;
	chunked_ = false;
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
			case Done :
				loop = -1;
				break ;
		}
	}
	return (external_state_);
}

bool HttpParser::IsOWS(char c)
{
	return (c == ' ' || c == '\t');
}

std::string HttpParser::TrimOWS(const std::string& s)
{
	size_t start = 0;
	while (start < s.size() && IsOWS(s[start]))
		start++;
	size_t end = s.size();
	while (end > start && IsOWS(s[end - 1]))
		end--;
	return s.substr(start, end - start);
}

std::string HttpParser::ToLower(std::string s)
{
	for (size_t i = 0; i < s.size(); i++)
	{
		unsigned char c = static_cast<unsigned char>(s[i]);
		if (c >= 'A' && c <= 'Z')
			s[i] = static_cast<char>(c - 'A' + 'a');
	}
	return s;
}

bool HttpParser::ParseDecimalSize(const std::string& s, size_t* out)
{
	if (!out)
		return false;
	std::string t = TrimOWS(s);
	if (t.empty())
		return false;
	size_t n = 0;
	for (size_t i = 0; i < t.size(); i++)
	{
		unsigned char c = static_cast<unsigned char>(t[i]);
		if (c < '0' || c > '9')
			return false;
		size_t digit = static_cast<size_t>(c - '0');
		if (n > (std::numeric_limits<size_t>::max() - digit) / 10)
			return false;
		n = n * 10 + digit;
	}
	*out = n;
	return true;
}

int HttpParser::ParseHeaders()
{
	// Wait until we have the full header section.
	input_i_ = input_buffer_.find("\r\n\r\n");
	if (input_i_ == std::string::npos)
		return (-1);

	std::string header_block = input_buffer_.substr(0, input_i_);
	// Consume header block + CRLFCRLF.
	input_buffer_ = input_buffer_.substr(input_i_ + 4);

	headers_.clear();
	body_expected_len_ = 0;
	chunked_ = false;

	// Parse line by line (CRLF separated)
	size_t pos = 0;
	while (pos < header_block.size())
	{
		size_t eol = header_block.find("\r\n", pos);
		std::string line;
		if (eol == std::string::npos)
		{
			line = header_block.substr(pos);
			pos = header_block.size();
		}
		else
		{
			line = header_block.substr(pos, eol - pos);
			pos = eol + 2;
		}

		if (line.empty())
			continue;

		// Obsolete line folding not supported.
		if (!line.empty() && (line[0] == ' ' || line[0] == '\t'))
		{
			state_ = Error;
			external_state_ = InvalidRequest;
			return (-1);
		}

		size_t colon = line.find(':');
		if (colon == std::string::npos)
		{
			state_ = Error;
			external_state_ = InvalidRequest;
			return (-1);
		}

		std::string name = line.substr(0, colon);
		std::string value = line.substr(colon + 1);
		name = TrimOWS(name);
		value = TrimOWS(value);
		if (name.empty())
		{
			state_ = Error;
			external_state_ = InvalidRequest;
			return (-1);
		}
		for (size_t i = 0; i < name.size(); i++)
		{
			if (!is_token_char(static_cast<unsigned char>(name[i])))
			{
				state_ = Error;
				external_state_ = InvalidRequest;
				return (-1);
			}
		}

		std::string name_l = ToLower(name);
		if (headers_.find(name_l) != headers_.end())
			headers_[name_l] += "," + value;
		else
			headers_[name_l] = value;
	}

	// Determine body semantics (no chunked parsing yet, but detect it).
	std::map<std::string, std::string>::const_iterator it;
	it = headers_.find("transfer-encoding");
	if (it != headers_.end())
	{
		std::string te = ToLower(it->second);
		// Minimal detection: if it contains "chunked", treat as chunked.
		if (te.find("chunked") != std::string::npos)
			chunked_ = true;
	}

	it = headers_.find("content-length");
	if (it != headers_.end())
	{
		size_t len = 0;
		if (!ParseDecimalSize(it->second, &len))
		{
			state_ = Error;
			external_state_ = InvalidRequest;
			return (-1);
		}
		body_expected_len_ = len;
	}

	// If both TE: chunked and Content-Length are present, we don't implement it yet.
	if (chunked_)
	{
		state_ = Error;
		external_state_ = InvalidRequest;
		return (-1);
	}

	if (body_expected_len_ == 0)
	{
		body_.clear();
		external_state_ = Complete;
		state_ = Done;
		return (-1);
	}

	state_ = Body;
	return (0);
}

int HttpParser::ParseBody()
{
	// We only support Content-Length bodies for now.
	if (input_buffer_.size() < body_expected_len_)
		return (-1);
	body_ = input_buffer_.substr(0, body_expected_len_);
	input_buffer_ = input_buffer_.substr(body_expected_len_);
	external_state_ = Complete;
	state_ = Done;
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
	// i + 1 to skip the first space.
	i++;
	j = input_buffer_.find(" ", i);
	if (j == std::string::npos)
	{
		state_ = Error;
		external_state_ = InvalidRequest;
		return (-1);
	}
	// Hard copy the path since we don't need to check anything
	// j - i: Second parameter of substr is length of the string.
	// TODO: What if input is: GET  HTTP/1.0
	// You have no path, only whitespace. How do we deal with this?
	path_ = input_buffer_.substr(i, j - i);
	// Skip the space by j + 1
	j++;
	// input_i_ - j because that is the length of the rest of string.
	temp = input_buffer_.substr(j, input_i_ - j);
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
	std::cout << "Body: " << body_ << "\n";
	std::cout << "Input Buffer: " << input_buffer_;
	std::cout << std::endl;
}
