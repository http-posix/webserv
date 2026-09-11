#pragma once
#include <string>
#include <map>

#include "config/parser/parser.hpp"
#include "http/request/request.hpp"

enum ResponseStatusCode
{
	BadRequest = 400,
	FileNotFound = 404,
	UnsupportedMediaType = 415,
};

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

	void HandleGet(HttpRequest& req, const ServerConfig* cfg);
	//void HandlePost(HttpRequest& req, ServerConfig& cfg);
	//void HandleDelete(HttpRequest& req, ServerConfig& cfg);

	public :

	class HttpResponseException : public std::exception {
		private :
			int error_code_;
		public :
			HttpResponseException(int code) throw() : error_code_(code) {};
			int GetErrorCode() { return (error_code_); };
	};


	HttpResponse(HttpRequest req, const ServerConfig* cfg);
	// Serializes the response into a raw HTTP/1.1 wire format string
	std::string	Serialize() const;
};
