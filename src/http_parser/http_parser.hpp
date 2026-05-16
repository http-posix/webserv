#include <string>
#include <map>
#include "../http_request/http_request.hpp"

class HttpParser
{
	private:
		std::string input_buffer_;
		size_t input_i_;
		HttpParserIntState state_;
		HttpParserState external_state_;
		
		HttpMethod method_;
		std::string path_;
		std::string version_;
		std::map<std::string, std::string> headers_;
		std::string body_;
		
		int ParseRequestLine();
		int ParseMethod(std::string str);
		int ParseVersion(std::string str);
		int ParseHeaders();
		int ParseBody();
	public:
		HttpParser(void);
		HttpParser(const HttpParser& other);
		HttpParser& operator=(const HttpParser& other);
		~HttpParser(void);

		HttpParserState Feed(const char *str);
};
