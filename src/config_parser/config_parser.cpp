#include "../app_exception/app_exception.hpp"
#include "../config_tokenizer/config_tokenizer.hpp"
#include "config_parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <cstdint>

/* ========================================================================== */
/*                          Constructors & Destructors                        */
/* ========================================================================== */

ConfigParser::ConfigParser(void) : tokenizer_("")
{
}

ConfigParser::ConfigParser(const std::string& filename) : tokenizer_("")
{
	if (readFile(filename) != 0)
		throw ConfigException("Failed to open configuration file: " + filename);
}

ConfigParser::~ConfigParser(void)
{
}

/* ========================================================================== */
/*                               Public Methods                               */
/* ========================================================================== */

int ConfigParser::readFile(const std::string& filename)
{
	std::ifstream		inputFile(filename.c_str());
	std::stringstream	buffer;

	if (!inputFile.is_open())
		return (-1);
	buffer << inputFile.rdbuf();
	file_string_ = buffer.str();
	if (file_string_.empty())
		return (-1);

	return (0);
}

int	ConfigParser::removeComments()
{
	if (file_string_.empty())
		return (-1);
	size_t start = file_string_.find("#");
	while (start != std::string::npos)
	{
		size_t end = file_string_.find("\n", start);
		// Erase from start for `end - start` characters.
		file_string_.erase(start, end - start);
		start = file_string_.find("#");
	}
	return (0);
}

std::vector<std::string> ConfigParser::collectUntil(const std::string& stop)
{
	std::vector<std::string> values;
	pos_ = tokenizer_.next();
	while (pos_.value != stop)
	{
		if (pos_.type == ConfigToken::EndOfFile)
			throw ConfigException("Unexpected end of file, expected '" + stop + "'.");
		values.push_back(pos_.value);
		pos_ = tokenizer_.next();
	}
	pos_ = tokenizer_.next();
	return (values);
}

void ConfigParser::expect(const std::string& value)
{
	if (pos_.value != value)
		throw ConfigException("Expected '" + value + "' but got '" + pos_.value + "'.");
	pos_ = tokenizer_.next();
}

std::string ConfigParser::join(const std::vector<std::string>& values)
{
	std::string result;
	for (size_t i = 0; i < values.size(); ++i)
		result += values[i];
	return (result);
}

int ConfigParser::parseInt(const std::string& value)
{
	std::size_t idx = 0;
	long result;
	try
	{
		result = std::stol(value, &idx);
	}
	catch (const std::exception&)
	{
		throw ConfigException("Invalid number '" + value + "'.");
	}
	if (idx != value.size())
		throw ConfigException("Invalid number '" + value + "'.");
	return (static_cast<int>(result));
}

size_t ConfigParser::parseSize(const std::string& value)
{
	std::size_t idx = 0;
	unsigned long result;
	try
	{
		result = std::stoul(value, &idx);
	}
	catch (const std::exception&)
	{
		throw ConfigException("Invalid size '" + value + "'.");
	}
	if (idx != value.size())
		throw ConfigException("Invalid size '" + value + "'.");
	return (static_cast<size_t>(result));
}

size_t ConfigParser::applyUnit(size_t value, const std::string& unit)
{
	std::string normalized = unit;
	for (size_t i = 0; i < normalized.size(); ++i)
		normalized[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(normalized[i])));

	if (normalized == "k" || normalized == "kb")
		return (value * 1024);
	if (normalized == "m" || normalized == "mb")
		return (value * 1024 * 1024);
	if (normalized == "g" || normalized == "gb")
		return (value * 1024 * 1024 * 1024);
	throw ConfigException("Unknown size unit '" + unit + "'.");
}

void	ConfigParser::createServerConfig()
{
	ServerConfig server_config;

	while (pos_.value != "}")
	{
		if (pos_.type == ConfigToken::EndOfFile)
			throw ConfigException("Unterminated 'server' block, expected '}'.");

		if (pos_.value == "listen")
		{
			pos_ = tokenizer_.next();
			if (pos_.type != ConfigToken::Number)
				throw ConfigException("Expected a port number after 'listen'.");
			int port = parseInt(pos_.value);
			if (port < 0 || port > 65535)
				throw ConfigException("Port out of range: '" + pos_.value + "'.");
			if (port == 0)
				throw ConfigException("Port 0 should not be used for legitimate purposes");
			server_config.listen_port.push_back(static_cast<uint16_t>(port));
			pos_ = tokenizer_.next();
			expect(";");
		}
		else if (pos_.value == "hostname")
		{
			server_config.hostname = join(collectUntil(";"));
			if (server_config.hostname.empty())
				throw ConfigException("Empty 'hostname' directive.");
		}
		else if (pos_.value == "root")
		{
			server_config.root = join(collectUntil(";"));
			if (server_config.root.empty())
				throw ConfigException("Empty 'root' directive.");
		}
		else if (pos_.value == "index")
		{
			server_config.index = join(collectUntil(";"));
			if (server_config.index.empty())
				throw ConfigException("Empty 'index' directive.");
		}
		else if (pos_.value == "client_max_body_size")
		{
			pos_ = tokenizer_.next();
			if (pos_.type != ConfigToken::Number)
				throw ConfigException("Expected a number after 'client_max_body_size'.");
			size_t result = parseSize(pos_.value);
			pos_ = tokenizer_.next();
			if (pos_.value != ";")
			{
				if (pos_.type != ConfigToken::Identifier)
					throw ConfigException("Expected ';' after 'client_max_body_size'.");
				result = applyUnit(result, pos_.value);
				pos_ = tokenizer_.next();
			}
			expect(";");
			server_config.client_max_body_size = result;
		}
		else if (pos_.value == "error_page")
		{
			pos_ = tokenizer_.next();
			if (pos_.type != ConfigToken::Number)
				throw ConfigException("Expected a status code after 'error_page'.");

			int code = parseInt(pos_.value);
			if (code < 400 || code > 599)
				throw ConfigException("Error page: " + pos_.value + " out of range (400-599)");

			ConfigToken temp_pos;
			temp_pos = tokenizer_.checkNext();
			if (temp_pos.type == ConfigToken::Number)
				throw ConfigException("Error number cannot be followed by another error number.");

			std::string path = join(collectUntil(";"));
			if (path.empty())
				throw ConfigException("Empty path in 'error_page' directive.");
			server_config.error_pages[code] = path;
		}
		else if (pos_.value == "location")
		{
			std::string uri = join(collectUntil("{"));
			if (uri.empty())
				throw ConfigException("Missing URI for 'location' directive.");
			LocationConfig location;
			location.uri_path = uri;
			createLocationConfig(location);
			server_config.locations.push_back(location);
		}
		else
		{
			throw ConfigException("Unexpected token '" + pos_.value + "' in 'server' block.");
		}
	}
	// Check if server block has necessary information to function.
	if (server_config.hostname.empty())
		server_config.hostname = "";
	if (server_config.listen_port.empty())
		throw (ConfigException("Server block requires at least one port!"));
	pos_ = tokenizer_.next();
	config_.servers.push_back(server_config);
}

void ConfigParser::createLocationConfig(LocationConfig& location)
{
	while (pos_.value != "}")
	{
		if (pos_.type == ConfigToken::EndOfFile)
			throw ConfigException("Unterminated 'location' block, expected '}'.");

		if (pos_.value == "methods")
		{
			std::vector<std::string> values = collectUntil(";");
			for (size_t i = 0; i < values.size(); ++i)
			{
				if (values[i] == ",")
					continue;
				if (values[i] == "GET" || values[i] == "POST" || values[i] == "DELETE")
					location.allowed_methods.push_back(values[i]);
				else
					throw (ConfigException(values[i] + " is not a valid method (GET/POST/DELETE)."));
			}
			if (location.allowed_methods.empty())
				throw ConfigException("Empty 'methods' directive.");
		}
		else if (pos_.value == "return")
		{
			pos_ = tokenizer_.next();
			if (pos_.type != ConfigToken::Number)
				throw ConfigException("Expected a status code after 'return'.");
			int code = parseInt(pos_.value);
			if (code < 300 || code > 399)
				throw ConfigException("Redirection code: " + pos_.value + "out of range (300-399)");
			std::string url = join(collectUntil(";"));
			if (url.empty())
				throw ConfigException("Empty URL in 'return' directive.");
			location.redirections = std::make_pair(code, url);
		}
		else if (pos_.value == "root")
		{
			location.root = join(collectUntil(";"));
			if (location.root.empty())
				throw ConfigException("Empty 'root' directive in location '" + location.uri_path + "'.");
		}
		else if (pos_.value == "index")
		{
			location.index = join(collectUntil(";"));
			if (location.index.empty())
				throw ConfigException("Empty 'index' directive in location '" + location.uri_path + "'.");
		}
		else if (pos_.value == "autoindex")
		{
			pos_ = tokenizer_.next();
			if (pos_.value == "on")
				location.autoindex = true;
			else if (pos_.value == "off")
				location.autoindex = false;
			else
				throw ConfigException("Expected 'on' or 'off' after 'autoindex'.");
			pos_ = tokenizer_.next();
			expect(";");
		}
		else if (pos_.value == "upload_enable")
		{
			pos_ = tokenizer_.next();
			if (pos_.value == "on")
				location.upload_enable = true;
			else if (pos_.value == "off")
				location.upload_enable = false;
			else
				throw ConfigException("Expected 'on' or 'off' after 'upload_enable'.");
			pos_ = tokenizer_.next();
			expect(";");
		}
		else if (pos_.value == "upload_store")
		{
			location.upload_location = join(collectUntil(";"));
			if (location.upload_location.empty())
				throw ConfigException("Empty 'upload_store' directive in location '" + location.uri_path + "'.");
		}
		else
		{
			throw ConfigException("Unexpected token '" + pos_.value + "' in location '" + location.uri_path + "'.");
		}
	}
	pos_ = tokenizer_.next();
}

void ConfigParser::parseFromString()
{
	if (file_string_.empty())
		throw ConfigException("Configuration file is empty.");
	removeComments();

	config_.servers.clear();
	tokenizer_ = ConfigTokenizer(file_string_);
	pos_ = tokenizer_.next();

	while (pos_.type != ConfigToken::EndOfFile)
	{
		if (pos_.type == ConfigToken::Keyword && pos_.value == "server")
		{
			pos_ = tokenizer_.next();
			if (pos_.type != ConfigToken::Symbol || pos_.value != "{")
				throw ConfigException("Expected '{' after 'server' keyword.");
			pos_ = tokenizer_.next();
			createServerConfig();
		}
		else
		{
			throw ConfigException("Unexpected token '" + pos_.value + "' outside a 'server' block.");
		}
	}

	if (config_.servers.empty())
		throw ConfigException("Configuration must contain at least one 'server' block.");
}

const Config& ConfigParser::getConfig() const
{
	return (config_);
}

