#include "http/router/router.hpp"
#include "http/http_status_code.hpp"
#include "utils/logger/logger.hpp"

#include <filesystem>

void HttpRouter::AppendIndex(const ConfigStruct* cfg)
{
	if (IsDir(rerouted_filepath_) == true)
	{
		if (!(rerouted_filepath_[rerouted_filepath_.size() - 1] == '/'))
			rerouted_filepath_ += "/";
		if (cfg != NULL)
		{
			if (!cfg->index.empty())
				rerouted_filepath_ += cfg->index;
			else
				LOG_DEBUG("Requested file is directory, but no index element is found in config.");
		}
	}
}

void HttpRouter::PrefixServerRoot()
{
	if (srv_cfg_.root.empty())
	{
		LOG_DEBUG("Server config does not have Index element.");
		return ;
	}
	rerouted_filepath_ = srv_cfg_.root + req_.path_;
};

void HttpRouter::ReplaceLocRoot()
{
	size_t pos = req_.path_.find(loc_->uri_path);
	rerouted_filepath_ = req_.path_;
	
	if (pos == std::string::npos)
		LOG_DEBUG("No location found in request!"); return ;
	rerouted_filepath_.replace(pos, loc_->uri_path.size(), loc_->root);
};

const LocationConfig* HttpRouter::FindLocation(const std::string& req_path,
		const std::vector<LocationConfig>& locations)
{
	const LocationConfig* result = nullptr;
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
};

void HttpRouter::HandleLocationRedirection()
{
	// If statement true, no redirection has been initialized.
	if (loc_->redirection.first == 0)
		return;
	switch (loc_->redirection.first)
	{
		case (MovedPermanently) : break ;
		case (Found) : break ;
		case (TemporaryRedirect) : break ;
		case (PermanentRedirect) : break ;
		default :
		{
			LOG_WARN("Redirection setting for location: " + loc_->uri_path + 
					" has unrecognized return value: " + 
					std::to_string(loc_->redirection.first) + ". Ignoring redirection.");
			return ;
		}
	}
	headers_["Location"] = loc_->redirection.second;
	throw (HttpStatusException(loc_->redirection.first));
};

// Iterate through an array of allowed methods.
// If no match is found between the requested 
// method and the methods allowed in the location
// an error is thrown (MethodNotAllowed)
void HttpRouter::HandleLocationMethod()
{
	std::string req_method_str;

	if (loc_->allowed_methods.empty())
		return ;
	switch (req_.method_)
	{
		case (Get) : req_method_str = "GET"; break;
		case (Post) : req_method_str = "POST"; break;
		case (Delete) : req_method_str = "DELETE"; break;
		default : req_method_str = "none";
	}

	for (size_t i = 0; i < loc_->allowed_methods.size(); i++)
	{
		if (req_method_str == loc_->allowed_methods[i])
			return ;
	}
	throw (HttpStatusException(MethodNotAllowed));
};

void HttpRouter::HandleLocationRoot()
{
	if (loc_->root.empty())
		PrefixServerRoot();
	else 
		ReplaceLocRoot();
};

bool HttpRouter::IsDir(std::string path)
{
	std::filesystem::path p = path;
	
	if (!std::filesystem::exists(p))
		throw (HttpStatusException(FileNotFound));
	if (std::filesystem::is_directory(p))
		return (true);
	return (false);
}

void HttpRouter::HandleLocationIndex()
{
	if (IsDir(rerouted_filepath_) != true)
		return ;
	if (loc_->index.empty())
	{
		if (loc_->autoindex == true)
			autoindex_ = true;
		else
			throw (HttpStatusException(Forbidden));
	}
	else
		AppendIndex(loc_);
};

void HttpRouter::HandleLocation()
{
	HandleLocationRedirection();
	HandleLocationMethod();
	HandleLocationRoot();
	HandleLocationIndex();
};

void HttpRouter::HandleNoLocation()
{
	PrefixServerRoot();
	AppendIndex(&srv_cfg_);
}

HttpRouter::HttpRouter(const HttpRequest& req, const ServerConfig& cfg) :
	req_(req), srv_cfg_(cfg), loc_(FindLocation(req.path_, cfg.locations))
{
	// Check whether the requested file path falls
	// under one of the locations set in the configuration.
	// Otherwise append index and set default root.
	try
	{
		if (loc_ != nullptr)
			HandleLocation();
		else
			HandleNoLocation();
	}
	// If a special status code has already found (Forbidden, File Not Found)
	// we can immediately return to create the response handling.
	catch (HttpStatusException& e)
	{
		http_status_code_ = e.GetErrorCode();
		BuildRoutedRequest();
		return;
	}

	if (cgi_ == true)
		BuildCgiRequest();
	else
		BuildRoutedRequest();
};
