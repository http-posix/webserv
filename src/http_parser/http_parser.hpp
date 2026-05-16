#include <string>
#include <map>
#include <cstddef>
#include <limits>
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
		size_t body_expected_len_;
		bool chunked_;
		
		int ParseRequestLine();
		int ParseMethod(std::string str);
		int ParseVersion(std::string str);
		int ParseHeaders();
		int ParseBody();

		static bool IsOWS(char c);
		static std::string TrimOWS(const std::string& s);
		static std::string ToLower(std::string s);
		static bool ParseDecimalSize(const std::string& s, size_t* out);
	public:
		HttpParser(void);
		HttpParser(const HttpParser& other);
		HttpParser& operator=(const HttpParser& other);
		~HttpParser(void);

		HttpParserState Feed(const char *str);
		void PrintParserRequest();
};
