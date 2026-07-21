#include "config_parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

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

void	ConfigParser::createServerConfig()
{
	pos = tokenizer_.Next();

	//TODO: I need to find an efficient way to check whether one of 
	// the keywords is present so that I can give specific 
	// instructions to set the variables for the server structure correctly
	//
	// TODO: I need to create a ServerConfig Structure. AKA:
	// I need to use the `new` keyword and add it to the vector
	// of the entire Config.

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

ConfigParser::ConfigParser(const std::string& filename) : tokenizer_(filename) {
}

ConfigParser::~ConfigParser(void) {
}

