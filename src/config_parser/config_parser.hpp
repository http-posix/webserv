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

struct ServerConfig
{
	// Non-virtual so we only have one hostname (ip adress)
	std::string hostname;

	// Ports to listen to. Multiple ports are allowed.
	// Ports are like gateways; our castle can have multiple entrances.
	std::vector<uint16_t> listen_port;

	// Html error pages to return in case of specific error pages.
	// Most browser have their own error page if non are provided,
	// however, subject defines we must give our own error_pages.
	std::pair<int, std::string> error_pages;

	// Settings for a specific location/directory
	std::vector<struct LocationConfig> locations;

	// Defines what the maximum size of a request should be.
	size_t client_max_body_size;
};

struct Config
{
	std::vector<struct ServerConfig> servers;
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
