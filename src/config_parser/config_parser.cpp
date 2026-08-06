#include "../config_tokenizer/config_tokenizer.hpp"
#include "config_parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
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
void	ConfigParser::createServerConfig()
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
	ServerConfig server_config;

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
	//TODO: I need to find an efficient way to check whether one of 
	// the keywords is present so that I can give specific 
	// instructions to set the variables for the server structure correctly
	//

	if (normalized == "k" || normalized == "kb")
		return (value * 1024);
	if (normalized == "m" || normalized == "mb")
		return (value * 1024 * 1024);
	if (normalized == "g" || normalized == "gb")
		return (value * 1024 * 1024 * 1024);
	throw ConfigException("Unknown size unit '" + unit + "'.");
}

	// Keywords to look for:
	// - listen
	// - server_name
	// - host
	// - root
	// - client_max_body_size
	// - index
	// - error_page
	// - location
	// - end of block `}`

	while (pos.value != "}")
	{
		if (pos.value == "port")
		{
		}
		else if (pos.value == "client_max_body_size")
		{
			size_t result;
			pos = tokenizer_.Next();
			if (pos.type == ConfigToken::Number)
			{
				result = std::stoul(pos.value);
				server_config.client_max_body_size = result;
			}
			else
			{
				// TODO: Unexpected token
			}
			pos = tokenizer_.Next();
			if (pos.value != ";")
			{
				//TODO: Unexpected token.
			}
			pos = tokenizer_.Next();
		}
		else if (pos.value == "error_page")
		{
			std::string result;
			pos = tokenizer_.Next();
			if (pos.type == ConfigToken::Number)
			{
				// TODO: How to set a variable for a pair data type?
				result = pos.value;
				// TODO: Convert string to integer.
			}
			else 
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
				// TODO: Throw error; unexpected token.
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
			if (pos.type == ConfigToken::String)
			{
				result = pos.value;
				// TODO: Pair path to given integer.
			}
			else
			{
				// TODO: Throw error for unexpected token.
			}
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

		// The following is an example of what we should do when we find one of the
		// eligible keywords. It is basically a ruleset/switchcase where we act on
		// specific keywords and set them to their corresponding values in our struct.
		else if (pos.value == "hostname")
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
	// TODO:
	// apend serverConfig to Config Structure.
}

void ConfigParser::createLocationConfig()
{
	pos = tokenizer_.Next();

	//- method
	//- return
	//- upload_store
	//- autoindex
	//- index
	//- cgi path
	//- `}` (end of location block)

}

void ConfigParser::parseFromString()
{
	removeComments();

	ConfigTokenizer tokenizer(file_string_);
	ConfigToken	pos;
	pos = tokenizer.Next();
	
	while (pos.type != ConfigToken::EndOfFile)
	{
		//TODO: Should we throw an error if we get a keyword that we
		// do not expect to be here? I think we should.
		if (pos.type == ConfigToken::Keyword && pos.value == "server")
		{
			pos = tokenizer.Next();
			//TODO; throw error.
			if (pos.value != "{")
				return ;
			createServerConfig();
		}
		pos = tokenizer.Next();
	}

	return ;
};
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
