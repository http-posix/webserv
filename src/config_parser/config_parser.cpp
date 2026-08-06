#include "../app_exception/app_exception.hpp"
#include "../config_tokenizer/config_tokenizer.hpp"
#include "config_parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <cstdint>

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
	pos = tokenizer_.Next();
	while (pos.value != stop)
	{
		if (pos.type == ConfigToken::EndOfFile)
			throw ConfigException("Unexpected end of file, expected '" + stop + "'.");
		values.push_back(pos.value);
		pos = tokenizer_.Next();
	}
	pos = tokenizer_.Next();
	return (values);
}

void ConfigParser::expect(const std::string& value)
{
	if (pos.value != value)
		throw ConfigException("Expected '" + value + "' but got '" + pos.value + "'.");
	pos = tokenizer_.Next();
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

	while (pos.value != "}")
	{
		if (pos.type == ConfigToken::EndOfFile)
			throw ConfigException("Unterminated 'server' block, expected '}'.");

		if (pos.value == "listen")
		{
			pos = tokenizer_.Next();
			if (pos.type != ConfigToken::Number)
				throw ConfigException("Expected a port number after 'listen'.");
			int port = parseInt(pos.value);
			if (port < 0 || port > 65535)
				throw ConfigException("Port out of range: '" + pos.value + "'.");
			server_config.listen_port.push_back(static_cast<uint16_t>(port));
			pos = tokenizer_.Next();
			expect(";");
		}
		else if (pos.value == "hostname")
		{
			server_config.hostname = join(collectUntil(";"));
			if (server_config.hostname.empty())
				throw ConfigException("Empty 'hostname' directive.");
		}
		else if (pos.value == "root")
		{
			server_config.root = join(collectUntil(";"));
			if (server_config.root.empty())
				throw ConfigException("Empty 'root' directive.");
		}
		else if (pos.value == "index")
		{
			server_config.index = join(collectUntil(";"));
			if (server_config.index.empty())
				throw ConfigException("Empty 'index' directive.");
		}
		else if (pos.value == "client_max_body_size")
		{
			pos = tokenizer_.Next();
			if (pos.type != ConfigToken::Number)
				throw ConfigException("Expected a number after 'client_max_body_size'.");
			size_t result = parseSize(pos.value);
			pos = tokenizer_.Next();
			if (pos.value != ";")
			{
				if (pos.type != ConfigToken::Identifier)
					throw ConfigException("Expected ';' after 'client_max_body_size'.");
				result = applyUnit(result, pos.value);
				pos = tokenizer_.Next();
			}
			expect(";");
			server_config.client_max_body_size = result;
		}
		else if (pos.value == "error_page")
		{
			pos = tokenizer_.Next();
			if (pos.type != ConfigToken::Number)
				throw ConfigException("Expected a status code after 'error_page'.");
			int code = parseInt(pos.value);
			std::string path = join(collectUntil(";"));
			if (path.empty())
				throw ConfigException("Empty path in 'error_page' directive.");
			server_config.error_pages[code] = path;
		}
		else if (pos.value == "location")
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
			throw ConfigException("Unexpected token '" + pos.value + "' in 'server' block.");
		}
	}
	pos = tokenizer_.Next();
	config_.servers.push_back(server_config);
}

void ConfigParser::createLocationConfig(LocationConfig& location)
{
	while (pos.value != "}")
	{
		if (pos.type == ConfigToken::EndOfFile)
			throw ConfigException("Unterminated 'location' block, expected '}'.");

		if (pos.value == "methods")
		{
			std::vector<std::string> values = collectUntil(";");
			for (size_t i = 0; i < values.size(); ++i)
			{
				if (values[i] == ",")
					continue;
				location.allowed_methods.push_back(values[i]);
			}
			if (location.allowed_methods.empty())
				throw ConfigException("Empty 'methods' directive.");
		}
		else if (pos.value == "return")
		{
			pos = tokenizer_.Next();
			if (pos.type != ConfigToken::Number)
				throw ConfigException("Expected a status code after 'return'.");
			int code = parseInt(pos.value);
			std::string url = join(collectUntil(";"));
			if (url.empty())
				throw ConfigException("Empty URL in 'return' directive.");
			location.redirections = std::make_pair(code, url);
		}
		else if (pos.value == "root")
		{
			location.root = join(collectUntil(";"));
			if (location.root.empty())
				throw ConfigException("Empty 'root' directive in location '" + location.uri_path + "'.");
		}
		else if (pos.value == "index")
		{
			location.index = join(collectUntil(";"));
			if (location.index.empty())
				throw ConfigException("Empty 'index' directive in location '" + location.uri_path + "'.");
		}
		else if (pos.value == "autoindex")
		{
			pos = tokenizer_.Next();
			if (pos.value == "on")
				location.autoindex = true;
			else if (pos.value == "off")
				location.autoindex = false;
			else
				throw ConfigException("Expected 'on' or 'off' after 'autoindex'.");
			pos = tokenizer_.Next();
			expect(";");
		}
		else if (pos.value == "upload_enable")
		{
			pos = tokenizer_.Next();
			if (pos.value == "on")
				location.upload_enable = true;
			else if (pos.value == "off")
				location.upload_enable = false;
			else
				throw ConfigException("Expected 'on' or 'off' after 'upload_enable'.");
			pos = tokenizer_.Next();
			expect(";");
		}
		else if (pos.value == "upload_store")
		{
			location.upload_location = join(collectUntil(";"));
			if (location.upload_location.empty())
				throw ConfigException("Empty 'upload_store' directive in location '" + location.uri_path + "'.");
		}
		else
		{
			throw ConfigException("Unexpected token '" + pos.value + "' in location '" + location.uri_path + "'.");
		}
	}
	pos = tokenizer_.Next();
}

void ConfigParser::parseFromString()
{
	if (file_string_.empty())
		throw ConfigException("Configuration file is empty.");
	removeComments();

	config_.servers.clear();
	tokenizer_ = ConfigTokenizer(file_string_);
	pos = tokenizer_.Next();

	while (pos.type != ConfigToken::EndOfFile)
	{
		if (pos.type == ConfigToken::Keyword && pos.value == "server")
		{
			pos = tokenizer_.Next();
			if (pos.type != ConfigToken::Symbol || pos.value != "{")
				throw ConfigException("Expected '{' after 'server' keyword.");
			pos = tokenizer_.Next();
			createServerConfig();
		}
		else
		{
			throw ConfigException("Unexpected token '" + pos.value + "' outside a 'server' block.");
		}
	}

	if (config_.servers.empty())
		throw ConfigException("Configuration must contain at least one 'server' block.");
}

const Config& ConfigParser::getConfig() const
{
	return (config_);
}

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
