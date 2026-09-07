#pragma once
#include <string>
#include <map>

#include "http/parser/parser.hpp"

// Simple response abstraction. Grows to support error pages,
// redirections, autoindex, CGI etc.
struct HttpResponse
{
	int			status_code;
	std::string status_text;
	std::map<std::string, std::string>	headers;
	std::string	body;

	// Serializes the response into a raw HTTP/1.1 wire format string
	std::string	Serialize() const;
};

// Dispatcher: decides which handler to run based on the request method.
// Grows into a Router (takes config, per-location settings, etc.)
HttpResponse	BuildResponse(const HttpParser& parser, const std::string& root);

// Individual HTTP method handlers.
HttpResponse	HandleGet(const std::string& path, const std::string& root);
// TODO(future): HandlePost, HandleDelete — POST uploads + deleted handling