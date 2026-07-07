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
	} type;
	std::string value;
};

class ConfigTokenizer
{
	public:
		explicit ConfigTokenizer(const std::string& input);
		ConfigToken Next();
	private:
		std::string src;
		size_t pos;
		bool isKeyword(std::string word);
};
