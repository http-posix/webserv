#include <fstream>
#include <sstream>
#include <unordered_map>
#include <sys/stat.h>

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

std::string HttpResponse::PrefixRoot(std::string uri, const ConfigStruct* cfg)
{
	std::string result;

	if (cfg != NULL)
	{
		if (cfg->root.empty())
			LOG_DEBUG("Accessing server/location structure without root");
		else
			result += cfg->root;
	}
	result += uri;
	return (result);
}

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

void HttpResponse::HandleLocationRedirection(const LocationConfig* loc)
{
	// Config parser checks that redirection numbers can only
	// be between 300 and 399. Thus this check is mainly to see
	// whether a redirection has been initialized (since the
	// default constructor initializes numbers to zero.)
	// Thus, if it is indeed 0, we know redirection is not
	// initialized.
	if (loc->redirection.first == 0)
		return ;
	switch (loc->redirection.first)
	{
		case (MovedPermanently) : break ;
		case (Found) : break ;
		case (TemporaryRedirect) : break ;
		case (PermanentRedirect) : break ;
		default :
		{
			LOG_WARN("Redirection setting for location: " + loc->uri_path + 
					" has unrecognized return value: " + 
					std::to_string(loc->redirection.first) + ". Ignoring redirection.");
			return ;
		}
	}
	LOG_DEBUG("Returning location: " + loc->redirection.second);
	headers_["Location"] = loc->redirection.second;
	throw (HttpResponseException(loc->redirection.first));
}

void HttpResponse::HandleGet(HttpRequest& req, const ServerConfig* serv_cfg)
{
	std::string file_path;
	const LocationConfig* location;

	//HandleLocation.
	location = FindLocation(req.path_, serv_cfg->locations);
	if (location != NULL)
	{
		LOG_DEBUG("Location: " + location->uri_path + " accessed for: " + req.path_);
		HandleLocationRedirection(location);
		HandleLocationMethod(req.method_, location);
		file_path = PrefixRoot(req.path_, location);
		file_path = AppendIndex(file_path, location);
	}
	else
	{
		file_path = PrefixRoot(req.path_, serv_cfg);
		file_path = AppendIndex(file_path, serv_cfg);
	}
	LOG_DEBUG("GET Request for file: " + file_path);
	// If Unsupported Media Type function will throw 415
	headers_["Content-Type"] = DetermineContentType(file_path);

	// If file cannot read, OpenFile will throw 404
	body_ = OpenFile(file_path);
}

std::string	HttpResponse::Serialize() const
{
	std::string	result = "HTTP/1.0 " + std::to_string(status_code_)
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

void HttpResponse::SetStatus(int status_code)
{
	status_code_ = status_code;
	switch (status_code)
	{
		case (OK) :	status_text_ = "OK"; break ;
		case (MovedPermanently) : status_text_ = "Moved Permanently"; break ;
		case (Found) : status_text_ = "Found"; break ;
		case (PermanentRedirect): status_text_ = "Permanent Redirect"; break ;
		case (TemporaryRedirect): status_text_ = "Temporary Redirect"; break ;
		case (BadRequest) : status_text_ = "Bad Request"; break ;
		case (FileNotFound) : status_text_ = "Not Found"; break ;
		case (MethodNotAllowed) : status_text_ = "Method Not Allowed"; break ;
		case (UnsupportedMediaType) : status_text_ = "Unsupported Media Type"; break ;
		default :
			{
				LOG_WARN("Error code: \'" + std::to_string(status_code) + "\' has not been implemented (yet)!");
				status_text_ = "Unknown Error"; break;
			}
	}
}

void HttpResponse::HardcodeErrorPage()
{
	std::string description;
	std::string title = std::to_string(status_code_) + " " + status_text_;

	switch (status_code_)
	{
		case (BadRequest) :
			description = "Your request was invalid. There are numerous explanations.";
			break ;
		case (FileNotFound) :
			description = "The file you tried to access could not be found or openend.";
			break ;
		case (MethodNotAllowed) :
			description = "You tried to either GET/POST/DELETE in a location that does not allow it.";
			break ;
		case (UnsupportedMediaType) :
			description = "The requested file has an extension that we do not support. (fe. .java)";
			break ;
		default :
			description = "Something went wrong. But we don't know what.";
			break ;
	}
	body_.clear();
	body_ += "<!DOCTYPE html>\n<html>\n";
	body_ +=  "<title>" + title + "</title>\n";
	body_ += "<body>\n";
	body_ += "<h1>" + title + "</h1>\n";
	body_ += "<p>" + description + "</p>\n";
	body_ += "</body>\n";
	body_ += "</html>";
}

void HttpResponse::HandleErrorPage(int status_code, const ServerConfig* cfg)
{
	SetStatus(status_code);
	headers_["Content-Type"] = "text/html";
	// In case some error occured while building the body, we need to clear it.
	// body_.clear();
	
	if (cfg != NULL && !cfg->error_pages.empty())
	{
		// .find() function returns an iterator specific to our unordered_map.
		//  It is easier to use auto instead of
		//  std::unordered_map<int, std::string>::iterator
		auto it = cfg->error_pages.find(status_code);
		// If we have a custom error page, try to open it.
		if (it != cfg->error_pages.end())
		{
			try
			{
				body_ = OpenFile(it->second);
				return ;
			}
			catch (HttpResponseException& e)
			{
				LOG_WARN("Custom Error Page cannot be opened; Falling back on built-in page.");
			}
		}
	}
	HardcodeErrorPage();
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
		int status_code = e.GetErrorCode();
		LOG_DEBUG("Request resulted in Code: " + std::to_string(status_code));
		//Handle redirection
		if (status_code > 299 && status_code < 400)
			SetStatus(status_code);
		// Handle error page
		else if (status_code > 399 && status_code < 500)
			HandleErrorPage(e.GetErrorCode(), cfg);
	}
};
