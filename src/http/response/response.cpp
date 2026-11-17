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

std::string	HttpResponse::ReadFile(std::string filename)
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

void	HttpResponse::UploadFile(std::string filename, std::string body)
{
	std::ofstream createdFile(filename.c_str(), std::ios::trunc);

	if (!createdFile.is_open())
	{
		LOG_DEBUG("Error in UploadFile for " + filename);
		throw (HttpResponseException(InternalServerError));
	}
	createdFile.write(body.data(), body.size());
	createdFile.close();
	if (!createdFile)
	{
		LOG_DEBUG("Error while uploading file for" + filename);
		throw (HttpResponseException(InternalServerError));
	}
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

void HttpResponse::HandleRedirection(std::string redirection)
{
	headers_["Location"] = redirection;
}

void HttpResponse::HandleErrorPage(int status_code)
{
	SetStatus(status_code);
	headers_["Content-Type"] = "text/html";
	// In case some error occured while building the body, we need to clear it.
	body_.clear();
	
	if (srv_cfg_ == nullptr)
		return ;
	if (srv_cfg_->error_pages.empty())
	{
		HardcodeErrorPage();
		return ;
	}
	// .find() function returns an iterator specific to our unordered_map.
	//  It is easier to use auto instead of
	//  std::unordered_map<int, std::string>::iterator
	auto it = srv_cfg_->error_pages.find(status_code);
	// If we have a custom error page, try to open it.
	if (it != srv_cfg_->error_pages.end())
	{
		try
		{
			body_ = ReadFile(it->second);
		}
		catch (HttpResponseException& e)
		{
			LOG_WARN("Custom Error Page cannot be opened; Falling back on built-in page.");
			HardcodeErrorPage();
		}
	}
}

void HttpResponse::HandleGet(const RoutedRequest& req)
{
	if (req.autoindex == true)
		HandleAutoIndex(req.uri);
	else
		body_ = ReadFile(req.uri);
	headers_["Content-Type"] = DetermineContentType(req.uri);
	LOG_DEBUG("GET Request for file: " + req.uri);
}

std::string CreateNewFilename()
{
	static int identifier = 1;
	std::string result = "upload-";

	result.append(std::to_string(identifier));
	// while file exists; increment identifier.
	return (result);
}

std::string ExtractFromContentDisposition(std::string str)
{
	std::string result;
	size_t pos = str.find("filename=");
	if (pos == std::string::npos)
		return "";
	pos += std::string("filename=").size();

	int quote = 0;
	size_t end = pos;
	while (end < str.size())
	{
		char c = str[end];
		if (quote != 0)
		{
			if (c == quote)
				break ;
		}
		else if (c == '"' || c == '\'')
			quote = c;
		else if (c == ';')
			break;
		end++;
	}
	if (quote != 0)
		result = str.substr(pos, end - pos);
	else 
		result = str.substr(pos + 1, end - pos + 1);
		
	return (result);
}

std::string FindFilename(std::map<std::string, std::string> headers)
{
	std::map<std::string, std::string>::iterator it;
	std::string content_disposition = "";
	std::string filename;
	for (it = headers.begin(); it != headers.end(); it++)
	{
		if (it->first == "content-disposition")
			content_disposition = it->second;
	}
	filename = ExtractFromContentDisposition(content_disposition);
	if (filename.empty())
		filename = CreateNewFilename();
	return (filename);
}

/*
1. Check if `upload_enable` flag is actually on.
2. Check if we have an upload location where we can actually upload.

3. Check if the upload location actually exists. 
4. Check if the upload location is an actual directory.

NB: 3 & 4 can be done in the config parser as well!

5. Get the header for `content-type`.
6. Get the header for `conten-disposition`.

7. Sanitize the `content-disposition` header into the filename.
8. Create filepath by upload_location + filename.
        Check if filepath is not empty. Otherwise Internal Server Error.

9. Upload the File to the server. Aka Upload the Payload to Filepath.

10. Set location header for response as Filepath. (request_path + filepath?)

11. Set Status as Created.
*/

void HttpResponse::HandlePost(const RoutedRequest& req)
{
	if (req.upload_enable == false)
		throw (HttpResponseException(Forbidden));
	if (req.upload_location.empty())
		throw(HttpResponseException(InternalServerError));
	std::string filename = FindFilename(req.headers);
	if (filename.empty())
		throw(HttpResponseException(InternalServerError));
	filename = req.upload_location + filename;

	UploadFile(filename, req.body);
	// Find the file name in headers.
	// open upload_location + filename
	// put body inside file
	// close file
	// setstatus 201 created. 
	// Create additional information if needed.
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
			case (Post) :
			{
				HandlePost(req);
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
