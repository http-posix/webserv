#include <fstream>
#include <sstream>
#include <unordered_map>
#include <sys/stat.h>
#include <filesystem>

#include "http/response/response.hpp"
#include "utils/logger/logger.hpp"
#include "config/structures.hpp"
#include "http/http_status_code.hpp"
#include "http/request.hpp"

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

std::string HttpResponse::DetermineContentType(const std::string& path)
{
	//find the last `.` starting from the right (reverse search).
	//copy that part into comparison
	const size_t dot = path.rfind('.');
	const size_t slash = path.rfind('/');
	// Second check is ../some_dir/./anotherfolder
	if (dot == std::string::npos || (slash != std::string::npos && slash > dot))
		return ("text/plain");
	const std::string extension = path.substr(dot);

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
	{
		std::string filename;
		const size_t temp_pos = path.rfind('/');
		if (temp_pos == std::string::npos)
			filename = path;
		else
			filename = path.substr(dot);
		headers_["Content-Disposition"] = "attachment";
		headers_["filename"] = "\"" + filename + "\"";
		return ("text/plain");
	}

	return (got->second);
}

void HttpResponse::HandleAutoIndex(std::string file_path)
{
	body_ += "<!DOCTYPE HTML><html><body>";
	body_ += "<h3> Directory Listing for: " + file_path + "</h3><ul>";
    if (file_path != "/")
      body_ += "<li><a href='../'>../</a></li>";
	try
	{
        if (!file_path.empty() && file_path.back() != '/')
			file_path += "/";
		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(file_path))
		{
			std::string name = entry.path().filename().string();
			if (entry.is_directory())
				name += "/";
			body_ += "<li><a href=\"" + name +"\">" + name + "</a></li>";
		}
	}
	catch (std::filesystem::filesystem_error& e)
	{
		LOG_WARN(e.what());
		throw (HttpResponseException(InternalServerError));
	}
	body_ += "</ul></body></html>\n";
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
		case (Forbidden) : status_text_ = "Forbidden"; break;
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
		case (Forbidden) :
			description = "We understand what you want, but we refuse to process it.";
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
	body_ += "</html>\n";
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

void HttpResponse::HandleGet(const RoutedRequest& req)
{
	if (req.autoindex == true)
		HandleAutoIndex(req.uri);
	else
		body_ = OpenFile(req.uri);
	headers_["Content-Type"] = DetermineContentType(req.uri);
	LOG_DEBUG("GET Request for file: " + req.uri);
}

void HttpResponse::BuildResponse(const RoutedRequest& req)
{
	try {
		if (req.http_status_code != -1)
			throw (HttpResponseException(req.http_status_code));
		switch (req.method)
		{
			case (Get) :
			{
				HandleGet(req);
				break ;
			}
			default :
				throw (HttpResponseException(BadRequest));
		}
		SetStatus(OK);
	}
	catch (HttpResponseException& e)
	{
		int status_code = e.GetErrorCode();
		status_code_ = status_code;
		if (status_code < 400 && status_code > 299)
			HandleRedirection(req.redirection);
		if (status_code > 399)
			HandleErrorPage(e.GetErrorCode());
		else
			SetStatus(status_code);
	}
	LOG_DEBUG("BuildResponse resulted in Code: " + std::to_string(status_code_));
}
