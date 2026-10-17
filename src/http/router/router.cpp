
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
