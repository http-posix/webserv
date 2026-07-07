#pragma once
#include <string>

struct ConfigToken
{
	enum TokenType
	{
		Keyword,
		Identifier,
		Symbol,
		Number,
		String,
		EndOfFile
	};
	std::string value;
};

class ConfigTokenizer
{ public:
		explicit ConfigTokenizer(const std::string& input);
		ConfigToken next();
	private:
		std::string src;
		size_t pos;
};
