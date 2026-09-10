#pragma once
#include <string>
#include <map>

#include "http/parser/parser.hpp"

// Simple response abstraction. Grows to support error pages,
// redirections, autoindex, CGI etc.
class HttpResponse
{
	private :
	int			status_code_;
	std::string status_text_;
	std::map<std::string, std::string>	headers_;
	std::string	body_;

	std::string OpenFile(std::string filename);
	std::string DetermineContentType(const std::string& path);
	void SetStatusOK();

	void HandleGet(HttpRequest& req, ServerConfig& cfg);
	//void HandlePost(HttpRequest& req, ServerConfig& cfg);
	//void HandleDelete(HttpRequest& req, ServerConfig& cfg);



	HttpResponse(HttpRequest req, ServerConfig& cfg);
	// Serializes the response into a raw HTTP/1.1 wire format string
	std::string	Serialize() const;
};
