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

ConfigParser::ConfigParser(void) {
	
}

ConfigParser::ConfigParser(const ConfigParser& other) {
	*this = other;
}

ConfigParser& ConfigParser::operator=(const ConfigParser& other) {
	if (this != &other) {
		
	}
	return *this;
}

ConfigParser::~ConfigParser(void) {
}

