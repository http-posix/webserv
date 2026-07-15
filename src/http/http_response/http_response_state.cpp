#include "http_response_state.hpp"

StatusCode check_file_error(const HttpRequest &request);
std::vector<unsigned char> read_file(const HttpRequest request);
StatusCode check_method_error(const HttpRequest &request);
std::string parse_type(const HttpRequest &request);

HttpResponseState::HttpResponseState() {}

// HttpResponseState::HttpResponseState(const HttpResponseState &src) {}

// HttpResponseState& HttpResponseState::operator=(const HttpResponseState &src) {}

HttpResponseState::~HttpResponseState() {}

void HttpResponseState::set_body(const HttpRequest &request)
{

	std::vector<unsigned char> buffer;

	buffer = read_file(request);

	// data() allows us to print the body's content,
	// by returning a pointer to the first char of the vector
	body_.assign(buffer.begin(), buffer.end());
	// std::cout << body_.data() << std::endl; // TODO: .data() is dangerous if buffer is not null terminated. overload print instead.
}

void HttpResponseState::set_statusCode(const HttpRequest &request)
{
	StatusCode status;

	status = check_method_error(request);
	if (status == OK)
		status = check_file_error(request);
	statusCode_ = status;
}

StatusCode HttpResponseState::get_statusCode()
{
	return statusCode_;
}

void HttpResponseState::set_headers(const HttpRequest &request)
{
	// store content length
	add_header("Content-Length", std::to_string(body_.size()));
	// store content type

	std::string type = parse_type(request);
	add_header("Content-Type", type);
}

void HttpResponseState::add_header(const std::string &name, const std::string &value)
{
	headers_.push_back({name, value});
}

static std::string convert_statusCode(int code) {
	if (code == 200)
		return "OK";
	else if (code == 403)
		return "FORBIDDEN";
	else if (code == 404)
		return "NOT FOUND";
	else if (code == 405)
		return "METHOD NOT ALLOWED";	
	return "CODE_READ_ERR"; // change default to application/octet-stream or 500 internal server?
}

// An HTTP response consists of 4 parts:
// 1. Status line (HTTP version, status code) -> the answer (success/fail)
// 2. Headers (key-value pairs, one per line to let the client know how to handle the body) -> the instructions (how to)
// 3. Blank line (separating headers from body)
// 4. Body (actual content) -> the content (what)

std::string HttpResponseState::serialize(void)
{
	std::string response;
	int code = static_cast<int>(this->statusCode_);
	std::string status_str = convert_statusCode(code);

	// first part: status line (HTTP version, status code)
	response.append(httpVersion_);
	response.append(" ");
	response.append(std::to_string(code));
	response.append(" ");
	response.append(status_str);
	response.append("\r\n");

	// second part: Headers
	for (const auto &h : headers_)
	{
		response.append(h.first);
		response.append(": ");
		response.append(h.second);
		response.append("\r\n");
	}

	// Default blank line separating headers from body
	response.append("\r\n");

	// Third part: body
	if (!body_.empty())
	{
		response.append(reinterpret_cast<const char *>(body_.data()), body_.size());
	}
	//std::cout << response << std::endl;
	return response;
}

int	HttpResponseState::fill_response(const HttpRequest &request) 
{
	set_statusCode(request);
	set_body(request);
	set_headers(request);
	serialize();

	return (0);
}