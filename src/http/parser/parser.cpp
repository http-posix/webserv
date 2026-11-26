#include "http/parser/parser.hpp"
#include "http/request.hpp"
#include "utils/logger/logger.hpp"

#include <iostream>
#include <vector>
#include <limits>
#include <cstring>

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

HttpParser::HttpParser(size_t client_max_body_size)
{
	method_ = None;
	state_ = RequestLine;
	external_state_ = NeedMoreData;
	body_expected_len_ = 0;
	client_max_body_size_ = client_max_body_size;
	chunked_ = false;
	// These values are only used when Transfer-Encoding: chunked is present.
	chunk_state_ = ChunkSize;
	chunk_size_ = 0;
}

//HttpParser::HttpParser(const HttpParser& other)
//{
//	*this = other;
//}

// HttpParser& HttpParser::operator=(const HttpParser& other)
// {
// 	if (this != &other)
// {
// 		
// 	}
// 	return (*this);
// }

HttpParser::~HttpParser(void)
{
}

HttpParserState HttpParser::Feed(const char *str)
{
	// This overload assumes a NUL-terminated string.
	return (Feed(str, std::strlen(str)));
}

HttpParserState HttpParser::Feed(const char *str, size_t length)
{
	// append() copies exactly length bytes, so chunk data may contain '\0'.
	input_buffer_.append(str, length);
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

bool HttpParser::ParseHexSize(const std::string& s, size_t* out)
{
	if (!out || s.empty())
		return false;
	size_t n = 0;
	for (size_t i = 0; i < s.size(); i++)
	{
		unsigned char c = static_cast<unsigned char>(s[i]);
		size_t digit;
		if (c >= '0' && c <= '9')
			digit = static_cast<size_t>(c - '0');
		else if (c >= 'a' && c <= 'f')
			digit = static_cast<size_t>(c - 'a' + 10);
		else if (c >= 'A' && c <= 'F')
			digit = static_cast<size_t>(c - 'A' + 10);
		else
			return false;
		// Chunk sizes are hexadecimal, but the overflow check works the same way
		// as the one in ParseDecimalSize.
		if (n > (std::numeric_limits<size_t>::max() - digit) / 16)
			return false;
		n = n * 16 + digit;
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
		LOG_DEBUG("(Invalid Request) OWS found at start of header line");
		state_ = Error;
		external_state_ = InvalidRequest;
		return (-1);
	}
	size_t colon = header_line.find(":");
	if (colon == std::string::npos)
	{
		LOG_DEBUG("(Invalid Request) No colon found in header");
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
		LOG_DEBUG("(Invalid Request): Empty header found");
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
		LOG_DEBUG("(Invalid Request): OWS found in header name");
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
		LOG_DEBUG("(Invalid Request): Non-token characters found in header name");
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
		std::string te = ToLower(TrimOWS(it->second));
		// Only plain "chunked" is supported. Other codings (for example gzip)
		// would need another decoder, so we reject them instead of guessing.
		if (te != "chunked")
		{
			LOG_DEBUG("(Invalid Request): transfer-encoding header with non-chunked value");
			state_ = Error;
			external_state_ = InvalidRequest;
			return (-1);
		}
		chunked_ = true;
		LOG_DEBUG("Request with \"Transfer-Encoding: chunked\" received");
	}

	// We do a similar thing for the content-length (when we have a body)
	it = headers_.find("content-length");
	if (it != headers_.end())
	{
		// A request with both headers is ambiguous. RFC says Transfer-Encoding
		// wins, but rejecting it is simpler and keeps the parser strict.
		if (chunked_)
		{
			LOG_DEBUG("(Invalid Request): Both Content-Length and Transfer-Encoding in request");
			state_ = Error;
			external_state_ = InvalidRequest;
			return (-1);
		}
		size_t len = 0;
		// In case we have an invalid entry for content-length we result in an error.
		if (!ParseDecimalSize(it->second, &len))
		{
			LOG_DEBUG("(Invalid Request): Error in parsing DecimalSize of Chunked HeaderLine");
			state_ = Error;
			external_state_ = InvalidRequest;
			return (-1);
		}
		body_expected_len_ = len;
		if (client_max_body_size_ > 0 && body_expected_len_ > client_max_body_size_)
		{
			LOG_DEBUG("Length of request larger than allowed max");
			state_ = Error;
			external_state_ = ContentLengthTooLarge;
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
	// For chunked requests there is no Content-Length. The terminating
	// "0\r\n\r\n" still has to be read before the request is complete.
	if (chunked_)
	{
		body_.clear();
		chunk_state_ = ChunkSize;
		chunk_size_ = 0;
	}
	state_ = Body;
	return (0);
}

int HttpParser::ParseBody()
{
	// Chunked bodies have their own small state machine because they arrive as
	// several pieces instead of one block with a known length.
	if (chunked_)
		return (ParseChunkedBody());
	if (input_buffer_.size() < body_expected_len_)
	{
		return (-1);
	}
	if (input_buffer_.size() > body_expected_len_)
	{
		LOG_DEBUG("(Invalid Request): Body of request bigger than Expected Length!");
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

int HttpParser::ParseChunkedBody()
{
	if (chunk_state_ == ChunkSize)
	{
		size_t eol = input_buffer_.find("\r\n");
		if (eol == std::string::npos)
			return (-1);

		std::string line = input_buffer_.substr(0, eol);
		// Chunk extensions start with ';'. We do not need them to read the body.
		size_t extension = line.find(';');
		if (extension != std::string::npos)
			line = line.substr(0, extension);
		line = TrimOWS(line);
		if (!ParseHexSize(line, &chunk_size_))
		{
			LOG_DEBUG("(Invalid Request): Couldn't parse heximal size of Chunked Body");
			state_ = Error;
			external_state_ = InvalidRequest;
			return (-1);
		}
		input_buffer_ = input_buffer_.substr(eol + 2);
		if (client_max_body_size_ > 0
			&& chunk_size_ > client_max_body_size_ - body_.size())
		{
			// Reuse the existing "too large" state to get the same 413 response
			// that Content-Length bodies already produce.
			LOG_DEBUG("Chunked body larger than allowed max");
			state_ = Error;
			external_state_ = ContentLengthTooLarge;
			return (-1);
		}
		chunk_state_ = chunk_size_ == 0 ? Trailer : ChunkData;
		return (0);
	}
	if (chunk_state_ == ChunkData)
	{
		if (input_buffer_.size() < chunk_size_)
			return (-1);
		// append() with a length keeps binary data intact.
		body_.append(input_buffer_.data(), chunk_size_);
		input_buffer_ = input_buffer_.substr(chunk_size_);
		chunk_state_ = ChunkDataEnd;
		return (0);
	}
	if (chunk_state_ == ChunkDataEnd)
	{
		if (input_buffer_.size() < 2)
			return (-1);
		if (input_buffer_.compare(0, 2, "\r\n") != 0)
		{
			state_ = Error;
			external_state_ = InvalidRequest;
			return (-1);
		}
		// Every chunk ends with CRLF after its data.
		input_buffer_ = input_buffer_.substr(2);
		chunk_state_ = ChunkSize;
		return (0);
	}
	// Trailers use the same line format as headers. An empty line ends them,
	// and therefore ends the whole chunked body.
	size_t eol = input_buffer_.find("\r\n");
	if (eol == std::string::npos)
		return (-1);
	// Last trailer signifies the end of the Chunked Request
	// It is signified by having an "empty" eol.
	// (aka only an eol)
	bool last_trailer = (eol == 0);
	input_buffer_ = input_buffer_.substr(eol + 2);
	if (!last_trailer)
		return (0);
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

int HttpParser::ParseRequestQuery(const std::string& target)
{
	// Only the FIRST '?' is a separator, any following '?' belongs to the query.
	// which we don't encode (for now). We might have to later, depends on Tetiana
	size_t separator = target.find('?');
	if (separator == std::string::npos)
	{
		path_ = target;
		query_.clear();
		return (0);
	}
	path_ = target.substr(0, separator);
	query_ = target.substr(separator + 1);
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
	{
		LOG_DEBUG("(Invalid Request): Couldn't parse method (" + temp + ")");
		state_ = Error;
		external_state_ = InvalidRequest;
		return (-1);
		// Invalid Method
	}
	size_t j;
	// i + 1 to skip the first space.
	i++;
	j = input_buffer_.find(" ", i);
	if (j == std::string::npos)
	{
		LOG_DEBUG("(Invalid Request): No space found between HTTP and Version");
		state_ = Error;
		external_state_ = InvalidRequest;
		return (-1);
	}
	// target == uri + query
	std::string target = input_buffer_.substr(i, j - i);
	if (ParseRequestQuery(target) == -1)
		return (-1);
	// Skip the space by j + 1
	j++;
	// input_i_ - j because that is the length of the rest of string.
	temp = input_buffer_.substr(j, input_i_ - j);
	// Trim Optional Whitespace left over from the path
	temp = TrimOWS(temp);
	if (ParseVersion(temp) == -1)
	{
		LOG_DEBUG("(Invalid Request): Version not Supported (" + temp + ")");
		state_ = Error;
		external_state_ = InvalidRequest; //TODO: Should really be 505
		return (-1);	
	}
	
	// Trim beginning of buffer to remove the request line. +2 to remove \r\n char as well.
	input_buffer_ = input_buffer_.substr(input_i_ + 2);
	state_ = Header;
	return (0);	
}

HttpRequest HttpParser::GetRequest()
{
	HttpRequest result;

	result.method_ = this->method_;
	result.path_ = this->path_;
	result.query_ = this->query_;
	result.version_ = this->version_;
	result.headers_ = this->headers_;
	result.body_ = this->body_;

	return (result);
}

int HttpParser::GetInternalState() const
{
	return (this->state_);
}

HttpParserState HttpParser::GetExternalState() const
{
	return (this->external_state_);
}

HttpMethod HttpParser::GetMethod() const
{
	return (this->method_);
}

std::string HttpParser::GetPath() const
{
	return (this->path_);
}

std::string HttpParser::GetQuery() const
{
	return (this->query_);
}

std::string HttpParser::GetVersion() const
{
	return (this->version_);
}

std::map<std::string, std::string> HttpParser::GetHeaders() const
{
	return (this->headers_);
}

std::string HttpParser::GetBody() const
{
	return (this->body_);
}

size_t HttpParser::GetBodyExpectedLength() const
{
	return (this->body_expected_len_);
}

bool HttpParser::GetChunked() const
{
	return (this->chunked_);
}

bool	HttpParser::HeadersComplete() const{
	return state_ == Body || state_ == Done;
}
