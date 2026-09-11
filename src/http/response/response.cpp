#include <fstream>
#include <sstream>
#include <unordered_map>

#include "http/response/response.hpp"
#include "http/parser/parser.hpp"
#include "utils/logger/logger.hpp"

std::string	HttpResponse::OpenFile(std::string filename)
{
	std::ifstream		inputFile(filename.c_str());
	std::stringstream	buffer;
	std::string			result;

	if (!inputFile.is_open())
	{
		LOG_WARN("Couldn't find file: " + filename);
		throw (HttpResponseException(FileNotFound));
	}
	buffer << inputFile.rdbuf();
	inputFile.close();
	result = buffer.str();
	if (result.empty())
	{
		return ("");
	}
	return (result);
}

const LocationConfig* HttpResponse::FindLocation(const std::string& path,
		const std::vector<LocationConfig>& locations)
{
	const LocationConfig* result = NULL;
	size_t longest_match = 0;

	// Iterate over the entire vector.
	for (std::vector<LocationConfig>::const_iterator i = locations.begin();
			i != locations.end(); i++)
	{
		// If we find the uri_path of the location
		// at the start of the filepath it means we have a match
		if (path.find(i->uri_path) == 0)
		{
			if (path.length() > longest_match)
			{
				longest_match = path.length();
				result = &(*i);
			}
		}
	}
	return (result);
}

std::string HttpResponse::DetermineContentType(const std::string& path)
{
	//find the last `.` starting from the right (reverse search).
	//copy that part into comparison
	const size_t pos = path.rfind('.');
	if (pos == std::string::npos)
		return ("text/plain");
	const std::string extension = path.substr(pos);

	static const std::unordered_map<std::string, std::string> content_types{
		{".mp4", "video/mp4"},
		{".mp3", "audio/mpeg"},
		{".html", "text/html"},
		{".png", "image/png"},
		{".jpg", "image/jpeg"}
	};
	const std::unordered_map<std::string, std::string>::const_iterator got = content_types.find(extension);

	if (got == content_types.end())
		throw (HttpResponseException(UnsupportedMediaType));

	return (got->second);
}

void HttpResponse::HandleGet(HttpRequest& req, const ServerConfig* cfg)
{
	std::string file_path;
	const LocationConfig* location;

	location = FindLocation(req.path_, cfg->locations);

	if (location != NULL)
	{
		// Throw error if invalid method.
		// HandleAllowedMethods(allowedmethods);
		LOG_DEBUG("Location: " + location->uri_path + " accessed for: " + req.path_);
		if (!location->root.empty())
			file_path += location->root;
		// Add custom index.html if location has this setting.
	}
	else
	{
		file_path = cfg->root;
	}
	// Append custom index if path is a directory.
	if (req.path_[req.path_.size() - 1] == '/' || req.path_.empty())
	{
		// TODO: Check if server has root.
		file_path += "/index.html";
	}
	else
		file_path += req.path_;

	LOG_DEBUG("GET Request for file: " + file_path);
	// If Unsupported Media Type function will throw 415
	headers_["Content-Type"] = DetermineContentType(file_path);

	// If file cannot read, OpenFile will throw 404
	body_ = OpenFile(file_path);
}

std::string	HttpResponse::Serialize() const
{
	std::string	result = "HTTP/1.1 " + std::to_string(status_code_)
		+ " " + status_text_ + "\r\n";

	for (const auto& [name, value] : headers_)
		result += name + ": " + value + "\r\n";
	result += "Content-Length: " + std::to_string(body_.size()) + "\r\n";
	result += "Connection: close\r\n";
	result += "\r\n" + body_;

	return (result);
}

void HttpResponse::SetStatusOK()
{
	status_code_ = 200;
	status_text_ = "OK";
}

HttpResponse::HttpResponse(HttpRequest req, const ServerConfig* cfg)
{
	try {
		switch (req.method_)
		{
			case (Get) :
			{
				HandleGet(req, cfg);
				break ;
			}
			default :
				throw (HttpResponseException(BadRequest));
		}
		SetStatusOK();
	}
	catch (HttpResponseException& e)
	{
		status_code_ = e.GetErrorCode();
		LOG_DEBUG("Invalid Request resulted in Code: " + std::to_string(status_code_));
		// TODO:
		// Handle error page
	}
};
