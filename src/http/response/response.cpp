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

const LocationConfig* HttpResponse::FindLocation(const std::string& req_path,
		const std::vector<LocationConfig>& locations)
{
	const LocationConfig* result = NULL;
	size_t longest_match = 0;

	for (size_t i = 0; i < locations.size(); i++)
	{
		std::string loc_path = locations[i].uri_path;
		size_t loc_len = loc_path.size();

		// Check if we find the location EXACTLY
		// at the start of the requested path.
		LOG_DEBUG(std::to_string(req_path.compare(0, loc_len, loc_path)));
		if (req_path.compare(0, loc_len, loc_path) == 0)
		{
			// If exact match we are dealing with
			// a directory request, still it is a valid request.
			bool exact_match = (req_path == loc_path);

			// A request `/data/images/picture1.jpq` with location `/data`
			// will correctly return `/data` as location since
			// it found a `/` after `/data` in the request path.
			bool req_has_slash = (req_path.size() > loc_len && req_path[loc_len] == '/');

			// If the request was `/data/images` and location is `/data/` it is
			// not an exact match but still a match, nor will we find a `/` at
			// req_path[loc_len] since it will one char too far.
			bool loc_ends_in_slash = (loc_len > 0 && loc_path[loc_len - 1] == '/');
			
			if (exact_match || loc_ends_in_slash || req_has_slash)
			{
				if (loc_len > longest_match || result == NULL)
				{
					longest_match = loc_len;
					result = &locations[i];
				}
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
		{".jpg", "image/jpeg"},
		{".gif", "image/gif"},
		{".css", "text/css"}
	};

	const std::unordered_map<std::string, std::string>::const_iterator got = content_types.find(extension);

	if (got == content_types.end())
		throw (HttpResponseException(UnsupportedMediaType));

	return (got->second);
}

void HttpResponse::HandleLocationMethod(enum HttpMethod &req_method, const LocationConfig* loc)
{
	std::string req_method_str;

	switch (req_method)
	{
		case (Get) : req_method_str = "GET"; break;
		case (Post) : req_method_str = "POST"; break;
		case (Delete) : req_method_str = "DELETE"; break;
		default : req_method_str = "none";
	}

	for (size_t i = 0; i < loc->allowed_methods.size(); i++)
	{
		if (req_method_str == loc->allowed_methods[i])
			return ;
	}
	throw (HttpResponseException(MethodNotAllowed));
}


void HttpResponse::HandleGet(HttpRequest& req, const ServerConfig* cfg)

std::string HttpResponse::AppendIndex(std::string uri, const ConfigStruct* cfg)
{
	struct stat stat_buf;
	std::string result = uri;

	if (stat(uri.c_str(), &stat_buf) == 0)
	{
		if (S_ISDIR(stat_buf.st_mode))
		{
			if (!(uri[uri.size() - 1] == '/'))
				result += "/";
			if (cfg != NULL)
			{
				if (!cfg->index.empty())
					return (result + cfg->index);
			}
			LOG_DEBUG("Requested file is directory, but no index element is found in config.");
			return (result + "index.html");
		}
	}
	return (result);
}
{
	std::string file_path;
	const LocationConfig* location;

	location = FindLocation(req.path_, cfg->locations);

	if (location != NULL)
	{
		LOG_DEBUG("Location: " + location->uri_path + " accessed for: " + req.path_);
		HandleLocationMethod(req.method_, location);
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
