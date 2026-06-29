#pragma once
#include <string>

class ConfigParser {
	private:
		std::string	file_string_;

	public:
		int readFile(const std::string& filename);

		ConfigParser(void);
		ConfigParser(const ConfigParser& other);
		ConfigParser& operator=(const ConfigParser& other);
		~ConfigParser(void);
};
