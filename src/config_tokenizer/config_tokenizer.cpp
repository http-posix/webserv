#include "config_tokenizer.hpp"
#include <string>

ConfigTokenizer::ConfigTokenizer(const std::string& input) 
	: src_(input), pos_(0) {}

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
	while (pos_ < src_.size() && std::isspace(src_[pos_])) ++pos_;

	// Check for End of File.
	if (pos_ >= src_.size()) return {ConfigToken::EndOfFile, ""};
	
	// If word starts with alphabetical char, assume Keyword or Identifier
	if (std::isalpha(src_[pos_]))
	{
		size_t start = pos_;
		while (pos_ < src_.size() && (std::isalnum(src_[pos_]) || src_[pos_] == '_' || src_[pos_] == '/')) ++pos_;
		std::string word = src_.substr(start, pos_ - start);
		if (isKeyword(word))
			return {ConfigToken::Keyword, word};
		else
			return {ConfigToken::Identifier, word};
	}

	// NB: We do not convert number (yet) in tokenizer
	if (std::isdigit(src_[pos_]))
	{
		size_t start = pos_;
		while (pos_ < src_.size() && isdigit(src_[pos_])) ++pos_;
		return {ConfigToken::Number, src_.substr(start, pos_ - start)};
	}

	if (src_[pos_] == '"' || src_[pos_] == '\'')
	{
		// Quote can either be ' or ", that's why we save it dynamically
		char quote = src_[pos_];
		size_t start = ++pos_;
		while (pos_ < src_.size() && src_[pos_] != quote)
			++pos_;
		std::string str = src_.substr(start, pos_ - start);
		if (pos_ < src_.size()) 
			++pos_;
		return {ConfigToken::String, str};
	}

	// Final call we check for things such as {, } ; etc.
	// (Single Character Tokens AKA Symbols)
	char ch = src_[pos_];
	// Skip char so we don't infinitely loop on it.
	++pos_;
	if (ch == '{' || ch == '}' || ch == ':')
		return {ConfigToken::Symbol, std::string(1, ch)};

	// As a failsafe, we assume final pos_sibility to be Identifier
	// Things such as ':' or '/' for paths or ports
	return {ConfigToken::Identifier, std::string(1, ch)};
}
