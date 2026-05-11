#include <string>
#include <map>

struct	HttpRequest
{
		std::string method_;
		std::string path_;
		std::string version_;
		std::string body_;
		std::map<std::string, std::string> headers_;
};
