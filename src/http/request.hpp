#pragma once
#include <string>
#include <map>

// Enums need to be in PascalCase. Is that only for the enum type or also for its contents?
enum HttpMethod
{
	None,
	Get,
	Post,
	Delete
};

struct CgiRequest
{
	std::string interpreter;	// e.g. /usr/bin/python3
	std::string script_path;	// resolved filesystem path of the script
	std::string script_name;	// URI of the request (path only, no query)
	std::string query_string;	// part after '?' (empty if none)
	std::string request_body;	// POST body, written to the script's stdin
};

struct RoutedRequest
{
	enum HttpMethod method;
	std::string uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;

	int http_status_code;
	bool autoindex;
	std::string redirection;
};

struct	HttpRequest
{
		enum HttpMethod method_;
		std::string path_;
		std::string version_;
		std::map<std::string, std::string> headers_;
		std::string body_;
};
