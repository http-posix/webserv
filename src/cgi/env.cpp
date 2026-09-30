#include "cgi/env.hpp"
#include "utils/logger/logger.hpp"

/* ========================================================================== */
/*                             Anonymous Namespace                            */
/* ========================================================================== */

namespace {
	// only Get and Post could reach this. Delete - ?
	std::string	FindRequestedMethod(HttpMethod method){
		if (method == HttpMethod::Get)
			return "GET";
		return "POST";
	}

	// RFC 3875 4.1.18: SHOULD NOT pass credentials to the script
	// "proxy" -> HTTP_PROXY is read as proxy config by many libs (httpoxy)
	bool	IsSafeHeader(const std::string& name){
		if (name == "authorization"
			|| name == "proxy-authorization"
			|| name == "proxy")
			return false;
		return true;
	}

	std::string HeaderToEnvName(const std::string& header)
	{
		std::string name = "HTTP_";
		name.reserve(name.size() + header.size());
		for (char c : header)
		{
			if (c == '-')
				name += '_';
			else if (c >= 'a' && c <= 'z')
				name += static_cast<char>(c - 'a' + 'A');
			else
				name += c;
		}
		return name;
	}
}

/* ========================================================================== */
/*                          Constructors & Destructors                        */
/* ========================================================================== */
CgiEnv::CgiEnv(const ConnInfo& info,
		const HttpRequest& req,
		const CgiInfo& cgi_info,
		const ServerConfig&,
		const std::string&)
{
	AddToEnv("GATEWAY_INTERFACE", kGatewayInterface);

	AddToEnv("QUERY_STRING", cgi_info.query_string);
	AddToEnv("REMOTE_ADDR", info.addr);
	AddToEnv("REQUEST_METHOD", FindRequestedMethod(req.method_));
	AddToEnv("SCRIPT_NAME", cgi_info.script_name);
	AddToEnv("SERVER_NAME", info.srv_name);
	AddToEnv("SERVER_PORT", info.srv_port);
	AddToEnv("SERVER_PROTOCOL", req.version_);
	AddToEnv("SERVER_SOFTWARE", kServerSoftware);


	AddToEnv("REMOTE_HOST", info.addr);
	AddHttpHeaders(req.headers_);

	if (!cgi_info.path_info.empty())
		AddToEnv("PATH_INFO", cgi_info.path_info);
}

/* ========================================================================== */
/*                               Public Methods                               */
/* ========================================================================== */

void	CgiEnv::AddToEnv(const std::string& key, const std::string& value){
	env.push_back(key + '=' + value);
}

void	CgiEnv::AddHttpHeaders(const std::map<std::string, std::string>& headers){
	for (const auto& [name, value] : headers){
		if (name == "content-length")
			AddToEnv("CONTENT_LENGTH", value);
		else if( name == "content-type")
			AddToEnv("CONTENT_TYPE", value);
		else if (IsSafeHeader(name))
			AddToEnv(HeaderToEnvName(name), value);
	}
}

std::vector<char*>	CgiEnv::BuildEnvCharP(){
	std::vector<char*>	envp;

	envp.reserve(env.size() + 1);
	for (std::string& s : env)
		envp.push_back(s.data());
	envp.push_back(nullptr);
	return envp;
}
