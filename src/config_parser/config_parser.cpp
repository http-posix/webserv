#include "config_parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
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
