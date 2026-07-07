#pragma once
#include <string>

class ConfigTokenizer
{ public:
		explicit ConfigTokenizer(const std::string& input);
		ConfigToken next();
	private:
		std::string src;
		size_t pos;
};
