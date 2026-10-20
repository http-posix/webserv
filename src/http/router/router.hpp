#pragma once

#include "config/structures.hpp"
#include "http/request.hpp"

#include <string>

/*
 * Think I need a structure that the Router can create
 * similar to a request but this time updated depending
 * on the location and server configuration.
 *
 * It also keeps track of whether CGI or a regular response
 * handling is needed.

struct RoutedRequest
{
	int status_code = -1;
	enum method;
		
	std::string uri;
*/

// Router follows the request to proper response handling. 
// It looks at the following:
// 	- Location handling/routing
// 	- Determine header content type
// 	- Root handling
// 	- Index handling
//
// It does not build a body!

class HttpRouter
{
	private :
		// References for easy access in Helper functions
		const HttpRequest& req_;
		const ServerConfig& srv_cfg_;
		const LocationConfig* loc_;


		// Private Variables that the Router will change.
		// Key Variables the Router creates.
		std::string rerouted_filepath_;
		bool autoindex_ = false;
		bool cgi_ = false;
		int http_status_code_;

		// Headers to copy for response generation
		std::map<std::string, std::string> headers_;
		
		// The two main handling of the router.
		void HandleLocation();
		void HandleNoLocation();

		// Two main creators for response generation. 
		// Depending on whether cgi is needed or not.
		void BuildCgiRequest();
		void BuildRoutedRequest();

		// Location specific helper functions
		const LocationConfig* FindLocation(const std::string& req_path,
				const std::vector<LocationConfig>& locations);
		void HandleLocationRedirection();
		void HandleLocationMethod();
		void HandleLocationRoot();
		void HandleLocationIndex();

		// ReRouting FilePath with respect to configuration.
		void ReplaceLocRoot();
		void PrefixServerRoot();
		void AppendIndex(const ConfigStruct* cfg);

		bool IsDir(std::string path);
	public :
		HttpRouter(const HttpRequest& req, const ServerConfig& cfg);

		class HttpStatusException : public std::exception {
			private :
				int error_code_;
			public :
				HttpStatusException(int code) throw() : error_code_(code) {};
				int GetErrorCode() { return (error_code_); };
		};

};
