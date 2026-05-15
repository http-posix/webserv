#include <string>
#include <map>

// Enums need to be in PascalCase. Is that only for the enum type or also for its contents?
enum HttpMethod
{
	None,
	Get,
	Post,
	Delete
};

enum HttpParserIntState
{
	Error,
	RequestLine,
	Header,
	Body
};
struct	HttpRequest
{
		enum HttpMethod method_;
		std::string path_;
		std::string version_;
		std::map<std::string, std::string> headers_;
		std::string body_;
};
