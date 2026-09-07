#include <fstream>
#include <sstream>

#include "http/response/response.hpp"
#include "http/parser/parser.hpp"

#include "utils/logger/logger.hpp"

std::string	openFile(std::string filename)
{
	std::ifstream		inputFile(filename.c_str());
	std::stringstream	buffer;
	std::string			result;

	if (!inputFile.is_open())
	{
		return ("");
	}
	buffer << inputFile.rdbuf();
	result = buffer.str();
	if (result.empty())
	{
		return ("");
	}
	return (result);
}

HttpResponse	BuildResponse(const HttpParser& parser, const std::string& root)
{
	switch (parser.GetMethod())
	{
		case Get:
			return HandleGet(parser.GetPath(), root);
		// TODO(future): Implement POST and DELETE handlers
		case Post:
			LOG_WARN("POST not implemented yet");
			break;
		case Delete:
			LOG_WARN("DELETE not implemented yet");
			break;
		case None:
			break;
	}
	HttpResponse	response;
	response.status_code = 501;
	response.status_text = "Not Implemented";
	response.headers["Content-Type"] = "text/plain";
	response.body = "501 Not Implemented";
	return response;
}

HttpResponse	HandleGet(const std::string& path, const std::string& root)
{
	HttpResponse	response;
	std::string		file_path = root;

	// TODO(future): Path resolution against config locations,
	//                redirections, index/autoindex, etc.
	if (path == "/" || path.empty())
		file_path += "/index.html";
	else
		file_path += path;

	response.status_code = 200;
	response.status_text = "OK";
	response.headers["Content-Type"] = "text/html";
	response.body = openFile(file_path);

	return response;
}

std::string	HttpResponse::Serialize() const
{
	std::string	result = "HTTP/1.1 " + std::to_string(status_code)
		+ " " + status_text + "\r\n";

	for (const auto& [name, value] : headers)
		result += name + ": " + value + "\r\n";
	result += "Content-Length: " + std::to_string(body.size()) + "\r\n";
	result += "Connection: close\r\n";
	result += "\r\n" + body;

	return result;
}