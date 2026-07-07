ConfigTokenizer::ConfigTokenizer(const std::string& input) 
	: src(input), pos(0) {}

bool ConfigTokenizer::isKeyword(std::string word)
{
	if (word == "server" || word == "location" || word == "listen" || word == "error_page" || word == "client_max_body_size" || word == "methods" || word == "return" || word == "root" || word == "autoindex" || word == "index" || word == "upload_enable" || word == "upload_store")
		return (true);
	return (false);
}
