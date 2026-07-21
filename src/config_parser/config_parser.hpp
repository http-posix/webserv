#pragma once
#include <string>
struct LocationConfig
{
	// URI: Uniform Resource Identifier -> path to the location
	std::string uri_path;

	// What methods are allowed in the (sub)directory
	// GET, POST, DELETE
	std::vector<std::string> allowed_methods;

	// Each return codenumber has their own redirection page
	// Somewhat similar to error_pages
	std::pair<int, std::string> redirections;

	// root (directory) to override default root of server
	// If non is provided, the server's default is used
	std::string root;

	// Location where uploaded files while be uploaded.
	std::string upload_location;

	// TODO
	// index
	// autoindex
};


class ConfigParser {
	private:
		std::string	file_string_;

	public:
		int readFile(const std::string& filename);
		int removeComments();

		ConfigParser(void);
		ConfigParser(const ConfigParser& other);
		ConfigParser& operator=(const ConfigParser& other);
		~ConfigParser(void);
};
