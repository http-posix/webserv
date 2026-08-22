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
		ConfigToken checkNext();
	private:
		std::string src_;
		size_t pos_;
		bool isKeyword(std::string word);
};
