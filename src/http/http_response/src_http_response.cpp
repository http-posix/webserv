#include "http_response_state.hpp"
#include <algorithm>

// read file:
// check if file exists,
// determine content type
// get file size to set length
// generate response object
// send it over to socket

// Generate HTTP response headers — You need to add:

// Status line: HTTP/1.1 200 OK\r\n
// Content-Type header (based on file extension)
// Content-Length header (from buffer size)
// Blank line to separate headers from body: \r\n
// Determine content type — Based on the file extension:

// .html → text/html
// .css → text/css
// .js → application/javascript
// etc.
// Build complete HTTP response — Combine status line + headers + body

// Send over socket — As mentioned in your comments

// if path does not exist -> NOT_FOUND
// if path exists but cannot be read → FORBIDDEN
// if path exists and is readable → OK

StatusCode	check_method_error(const HttpRequest &request)
{
	if (request.method_ == Get)
		return (OK);
	return (NOT_ALLOWED);
}

StatusCode	check_file_error(const HttpRequest &request)
{
	if (request.path_.empty())
		return (NOT_FOUND);
	std::error_code ec;
	const std::filesystem::path p(request.path_);
	if (!std::filesystem::exists(p, ec))
		return (NOT_FOUND);
	if (ec)
		return (FORBIDDEN);
	if (!std::filesystem::is_regular_file(p, ec))
		return (FORBIDDEN);
	if (ec)
		return (FORBIDDEN);
	std::ifstream input(p.c_str(), std::ifstream::binary);
	if (!input.is_open())
		return (FORBIDDEN);
	return (OK);
}

// read file into memory
std::vector<unsigned char> read_file(const HttpRequest request)
{
	// open file in binary mode
	std::ifstream input(request.path_, std::ifstream::binary);
	if (!input.is_open())
		return {};

	// check size of the file
	input.seekg(0, std::ios::end);
	std::streamsize size = input.tellg();
	if (size < 0)
		return {};

	input.seekg(0, std::ios::beg);
	// allocate memory
	std::vector<unsigned char> buffer(size);
	if (size > 0)
		input.read(reinterpret_cast<char *>(buffer.data()), size);
	input.close();
	return (buffer);
}
std::string complete_MIME_type(std::string extension) {
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	if (extension == "html")
    	return "text/html";
	else if (extension == "css")
		return "text/css";
	else if (extension == "json")
		return "application/json";
	else if (extension == "png")
		return "image/png";
	else if (extension == "jpg" || "jpeg" )
		return "image/jpeg";
	else if (extension == "txt")
		return "text/plain";
	return "application/octet-stream";
}
// extract content type from path;
// todo: add fall back in case of empty string (default MIME type)
std::string parse_type(const HttpRequest &request)
{
	std::filesystem::path p(request.path_);
	// extension() returns the substring starting at the last dot of the filename
	std::string str = p.extension().string();
	if (!str.empty() && str.front() == '.') {
		str.erase(0, 1);
		str = complete_MIME_type(str);
	}
	// std::cout << "here" << str << std::endl;
	return (str);
}