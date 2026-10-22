#pragma once
#include <string>
#include <map>

#include "config/structures.hpp"
#include "http/request.hpp"
#include "config/parser/parser.hpp"

// Simple response abstraction. Grows to support error pages,
// redirections, autoindex, CGI etc.
class HttpResponse
{
	private :
		const ServerConfig* srv_cfg_;

		int			status_code_ = -1;
		std::string status_text_;
		std::map<std::string, std::string>	headers_;
		std::string	body_;

		std::string OpenFile(std::string filename);
		std::string DetermineContentType(const std::string& path);
		void SetStatus(int status_code);

		void HardcodeErrorPage();
		void HandleErrorPage(int status_code);

		void HandleAutoIndex(std::string file_path);
		void HandleRedirection(std::string redirection);

		void HandleGet(const RoutedRequest& req);
		//void HandlePost();
		//void HandleDelete();
	public :
		class HttpResponseException : public std::exception {
			private :
				int error_code_;
			public :
				HttpResponseException(int code) throw() : error_code_(code) {};
				int GetErrorCode() { return (error_code_); };
		};
		
		void BuildResponse(const RoutedRequest& req);
		HttpResponse(const ServerConfig* cfg) : srv_cfg_(cfg){};
		// Serializes the response into a raw HTTP/1.1 wire format string
		std::string	Serialize() const;
};
