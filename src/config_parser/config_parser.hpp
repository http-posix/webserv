#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "../config_tokenizer/config_tokenizer.hpp"

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

	// Default index file to serve inside this location
	std::string index;

	// Whether to generate a directory listing when no index is present
	bool autoindex = false;

	// Whether file uploads are allowed in this location
	bool upload_enable = false;

	// Location where uploaded files while be uploaded.
	std::string upload_location;
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
	std::unordered_map<int, std::string> error_pages;

	// Default root (directory) used by every location that does not
	// provide its own.
	std::string root;

	// Default index file used by every location that does not provide its own.
	std::string index;

	// Settings for a specific location/directory
	std::vector<struct LocationConfig> locations;

	// Defines what the maximum size of a request should be.
	size_t client_max_body_size = 0;
};

struct Config
{
	std::vector<struct ServerConfig> servers;
};

class ConfigParser {
	private:
		Config config_;
		std::string	file_string_;
		ConfigTokenizer tokenizer_;
		ConfigToken pos;

		// Collects token values up to (and consuming) `stop`.
		// Throws ConfigException on end of file.
		std::vector<std::string> collectUntil(const std::string& stop);
		// Expects the current token to equal `value`, then advances.
		void expect(const std::string& value);
		static std::string join(const std::vector<std::string>& values);
		static int parseInt(const std::string& value);
		static size_t parseSize(const std::string& value);
	public:
		int readFile(const std::string& filename);
		int removeComments();
		void parseFromString();
		const Config& getConfig() const;
		void createServerConfig();
		void createLocationConfig();

		ConfigParser(const std::string& filename);
		~ConfigParser(void);
};
