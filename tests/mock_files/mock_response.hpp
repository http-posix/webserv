#pragma once

#include <string>

inline std::string BuildMockResponse() {
	const std::string body = "mock response: parser returned Complete\n";

	return "HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: " + std::to_string(body.size()) + "\r\n"
			"Connection: close\r\n"
			"\r\n" + body;
}
