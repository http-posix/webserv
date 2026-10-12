#pragma once
#include <string>
#include <map>
#include <cstddef>
#include "http/request/request.hpp"

enum HttpParserState
{
	NeedMoreData,
	Complete,
	InvalidRequest
};

class HttpParser
{
	private:
		enum IntState
		{
			Error,
			RequestLine,
			Header,
			Body,
			Done
		};

		std::string input_buffer_;
		size_t input_i_;
		IntState state_;
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
		int ParseSingleHeaderLine(const std::string& header_line);
		int ParseBody();

		static bool IsOWS(char c);
		static std::string TrimOWS(const std::string& s);
		static std::string ToLower(std::string s);
		static bool ParseDecimalSize(const std::string& s, size_t* out);
	public:
		HttpParser(void);
		~HttpParser(void);
		HttpParser(const HttpParser& other) = delete;
		HttpParser& operator=(const HttpParser& other) = delete;

		HttpParser(HttpParser&& other) noexcept = default;
		HttpParser& operator=(HttpParser&& other) noexcept = default;

		HttpParserState Feed(const char *str);

		HttpRequest GetRequest();

		//Getters for Testing, ResponseGen and Good Practice
		int GetInternalState() const;
		HttpParserState GetExternalState() const;
		HttpMethod GetMethod() const;
		std::string GetPath() const;
		std::string GetVersion() const;
		std::map<std::string, std::string> GetHeaders() const;
		std::string GetBody() const;
		size_t GetBodyExpectedLength() const;
		bool GetChunked() const;
};
