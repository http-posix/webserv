#pragma once

#include "config/structures.hpp"
#include "http/request.hpp"

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
		HttpRequest& _req;
		ServerConfig& _cfg_srv;
		LocationConfig& loc;


		
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

		// ReRouting FilePath with respect to configuration.
		std::string ReplaceRoot(std::string uri, const ConfigStruct* cfg);
		std::string PrefixRoot(std::string uri, const ConfigStruct* cfg);
		std::string AppendIndex(std::string uri, const ConfigStruct* cfg);

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
