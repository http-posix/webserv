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

static bool string_has_non_token_character(const std::string& str)
{
	for (size_t i = 0; i < str.size(); i++)
		{
			if (!is_token_char(static_cast<unsigned char>(str[i])))
				return (true);
		}
	return (false);
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

// TODO
// The requirements on HeaderLines are rather strict. 
// I believe the current implementation is insufficient and
// Needs further adaptation, along with a correct flowchart
int HttpParser::ParseSingleHeaderLine(const std::string& header_line)
{
	// HTTP Requests are rather specific.
	// We can have Optional WhiteSpace (OWS)
	// But it cannot be at the start of the header line...
	if (header_line[0] == ' ' || header_line[0] == '\t')
	{
		state_ = Error;
		external_state_ = InvalidRequest;
		return (-1);
	}
	size_t colon = header_line.find(":");
	if (colon == std::string::npos)
	{
		state_ = Error;
		external_state_ = InvalidRequest;
		return (-1);
	}
	std::string name = header_line.substr(0, colon);
	// +1 to skip the colon itself.
	std::string value = header_line.substr(colon + 1);
	name = TrimOWS(name);
	value = TrimOWS(value);
	// Empty name for header is invalid.
	if (name.empty())
	{
		state_ = Error;
		external_state_ = InvalidRequest;
		return (-1);
	}
	// Very specific but the name of a header is not allowed
	// to have any OWS in the header itself. Only around it.
	// I know this is weird to read, but if we do not NOT find whitespace
	// It means we DO have whitespace and the name is invalid.
	if (name.find(" ") != std::string::npos)
	{
		state_ = Error;
		external_state_ = InvalidRequest;
		return (-1);
	}
	// A header Name is not allowed to have any non-token characters
	// RFC 7230 explains which characters are (non) tokens.
	// TODO: This is a local function that I don't need anywhere else
	// and also is not a method. Therefore I kind of want it in a 
	// specific file called ParserUtils.cpp that is only
	// included by this specific file.
	if (string_has_non_token_character(name))
	{
		state_ = Error;
		external_state_ = InvalidRequest;
		return (-1);
	}

	name = ToLower(name);
	// If there is already a header with a similar name, append it.
	// Otherwise just add the value to the corresponding map.
	if (headers_.find(name) != headers_.end())
		headers_[name] += "," + value;
	else
		headers_[name] = value;
	return (0);
}

int HttpParser::ParseHeaders()
{
	input_i_ = input_buffer_.find("\r\n\r\n");
	if (input_i_ == std::string::npos)
		return (-1);

	// When we have the double CRLF we have a complete header section.
	// Extract the header section from the input:
	std::string header_block = input_buffer_.substr(0, input_i_);

	// Remove the header section from the buffer.
	// +4 for the double CRLF
	input_buffer_ = input_buffer_.substr(input_i_ + 4);

	size_t pos = 0;
	size_t eol;
	std::string header_line;
	headers_.clear();
	while (pos < header_block.size())
	{
		// Find End Of Line (single CRLF) starting from 'pos'
		eol = header_block.find("\r\n", pos);
		if (eol == std::string::npos)
		{
			// The final header line doesn't have a CRLF
			// Since we did not copy it beforehand...
			// So we copy everything starting from 'pos'
			header_line = header_block.substr(pos);
			// Make sure we can exit the while loop.
			pos = header_block.size();
		}
		else
		{
			// Copy string starting from 'pos' for 'eol - pos' characters
			header_line = header_block.substr(pos, eol - pos);
			// Move cursor past line
			// + 2 to move past CRLF
			pos = eol + 2;
		}
		// Some bimbo could have a header that is empty. Not invalid but a bit silly.
		if (header_line.empty())
			continue;
			
		if (ParseSingleHeaderLine(header_line) == -1)
			return (-1);
	}
	// Use a constant iterator from the map namespace to find transfer-encoding and set content-length
	std::map<std::string, std::string>::const_iterator it;
	// The .find() call will fall to .end() if no entry has been found.
	// Somewhat similar to std::string::npos
	it = headers_.find("transfer-encoding");
	if (it != headers_.end())
	{
		// Set value to lower just in case.
		// NB: We are not editing the value itself; we make a copy.
		std::string te = ToLower(it->second);
		// Minimal detection: if it contains "chunked", treat as chunked.
		if (te.find("chunked") != std::string::npos)
			chunked_ = true;
	}

	// We do a similar thing for the content-length (when we have a body)
	it = headers_.find("content-length");
	if (it != headers_.end())
	{
		size_t len = 0;
		// In case we have an invalid entry for content-length we result in an error.
		if (!ParseDecimalSize(it->second, &len))
		{
			state_ = Error;
			external_state_ = InvalidRequest;
			return (-1);
		}
		body_expected_len_ = len;
		// If both TE: chunked and Content-Length are present, we don't implement it yet.
		if (chunked_)
		{
			state_ = Error;
			external_state_ = InvalidRequest;
			return (-1);
		}
		// If the length of the body is 0, we done and we don't need to (try to) parse it.
		if (body_expected_len_ == 0)
		{
			body_.clear();
			external_state_ = Complete;
			state_ = Done;
			return (0);
		}
	}
	state_ = Body;
	return (0);
}

int HttpParser::ParseBody()
{
	// We only support Content-Length bodies for now.
	if (input_buffer_.size() < body_expected_len_)
	{
		return (-1);
	}
	if (input_buffer_.size() > body_expected_len_)
	{
		state_ = Error;
		external_state_ = InvalidRequest;
		return (-1);
	}
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
		// Method needs to be separated by a (single) space. 
		// If there is none, we have an invalid request. 
		// Example: GETHTTP 1.0
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
	std::cout << "Headers: " << "\n";
	for (const auto& pair : headers_)
        std::cout << pair.first << " = " << pair.second << "\n";
	std::cout << "Body: " << body_ << "\n";
	std::cout << "Input Buffer: " << input_buffer_;
	std::cout << std::endl;
}
