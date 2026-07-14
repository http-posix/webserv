#include "config_tokenizer.hpp"
#include <string>

ConfigTokenizer::ConfigTokenizer(const std::string& input) 
	: src(input), pos(0) {}

// Technically a helper function and does not need to be part of the class
bool ConfigTokenizer::isKeyword(std::string word)
{
	if (word == "server" || word == "location" || word == "listen" || word == "error_page" || word == "client_max_body_size" || word == "methods" || word == "return" || word == "root" || word == "autoindex" || word == "index" || word == "upload_enable" || word == "upload_store")
		return (true);
	return (false);
}

ConfigToken ConfigTokenizer::Next()
{
	// Skip Optional White Space (OWS)
	while (pos < src.size() && std::isspace(src[pos])) ++pos;

	// Check for End of File.
	if (pos >= src.size()) return {ConfigToken::EndOfFile, ""};
	
	// If word starts with alphabetical char, assume Keyword or Identifier
	if (std::isalpha(src[pos]))
	{
		size_t start = pos;
		while (pos < src.size() && (std::isalnum(src[pos]) || src[pos] == '_' || src[pos] == '/')) ++pos;
		std::string word = src.substr(start, pos - start);
		if (isKeyword(word))
			return {ConfigToken::Keyword, word};
		else
			return {ConfigToken::Identifier, word};
	}

	// NB: We do not convert number (yet) in tokenizer
	if (std::isdigit(src[pos]))
	{
		size_t start = pos;
		while (pos < src.size() && isdigit(src[pos])) ++pos;
		return {ConfigToken::Number, src.substr(start, pos - start)};
	}

	if (src[pos] == '"' || src[pos] == '\'')
	{
		// Quote can either be ' or ", that's why we save it dynamically
		char quote = src[pos];
		size_t start = ++pos;
		while (pos < src.size() && src[pos] != quote)
			++pos;
		std::string str = src.substr(start, pos - start);
		if (pos < src.size()) 
			++pos;
		return {ConfigToken::String, str};
	}

	// Final call we check for things such as {, } ; etc.
	// (Single Character Tokens AKA Symbols)
	char ch = src[pos];
	// Skip char so we don't infinitely loop on it.
	++pos;
	if (ch == '{' || ch == '}' || ch == ':')
		return {ConfigToken::Symbol, std::string(1, ch)};

	// As a failsafe, we assume final possibility to be Identifier
	// Things such as ':' or '/' for paths or ports
	return {ConfigToken::Identifier, std::string(1, ch)};
}
