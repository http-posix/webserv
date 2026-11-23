#pragma once
#include <string>
#include <map>
#include <cstddef>
#include "http/request.hpp"

enum HttpParserState
{
	NeedMoreData,
	Complete,
	InvalidRequest,
	ContentLengthTooLarge
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

		// A chunked body is read in small steps. Each step remembers which part
		// of the chunked format we are currently waiting for.
		enum ChunkState
		{
			ChunkSize,
			ChunkData,
			ChunkDataEnd,
			Trailer
		};

		std::string input_buffer_;
		size_t input_i_;
		IntState state_;
		HttpParserState external_state_;
		
		HttpMethod method_;
		std::string path_;
		std::string query_;
		std::string version_;
		std::map<std::string, std::string> headers_;
		std::string body_;
		size_t body_expected_len_;
		size_t client_max_body_size_;
		bool chunked_;
		ChunkState chunk_state_;
		size_t chunk_size_;

		int ParseRequestLine();
		int ParseMethod(std::string str);
		int ParseVersion(std::string str);
		int ParseRequestQuery(const std::string& target);
		int ParseHeaders();
		int ParseSingleHeaderLine(const std::string& header_line);
		int ParseBody();
		int ParseChunkedBody();

		static bool IsOWS(char c);
		static std::string TrimOWS(const std::string& s);
		static std::string ToLower(std::string s);
		static bool ParseDecimalSize(const std::string& s, size_t* out);
		static bool ParseHexSize(const std::string& s, size_t* out);
	public:
		HttpParser(size_t client_max_body_size = 0);
		~HttpParser(void);
		HttpParser(const HttpParser& other) = delete;
		HttpParser& operator=(const HttpParser& other) = delete;

		HttpParser(HttpParser&& other) noexcept = default;
		HttpParser& operator=(HttpParser&& other) noexcept = default;

		HttpParserState Feed(const char *str);
		// The length is needed because chunk data may contain '\0' bytes.
		// The older Feed() overload above is kept for string literals and tests.
		HttpParserState Feed(const char *str, size_t length);

		HttpRequest GetRequest();

		bool	HeadersComplete() const;

		//Getters for Testing, ResponseGen and Good Practice
		int GetInternalState() const;
		HttpParserState GetExternalState() const;
		HttpMethod GetMethod() const;
		std::string GetPath() const;
		std::string GetQuery() const;
		std::string GetVersion() const;
		std::map<std::string, std::string> GetHeaders() const;
		std::string GetBody() const;
		size_t GetBodyExpectedLength() const;
		bool GetChunked() const;
};
