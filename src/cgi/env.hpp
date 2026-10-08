/*
env variables

RFC -> must
	1 GATEWAY_INTERFACE - the dialect of CGI used by the server. CGI/1.1
	2 QUERY_STRING - URL-encoded search or parameter string. If the Script-URI has no query component, MUST be defined as an empty string ""
	3 REMOTE_ADDR - the network address of the client sending the request
	4 REQUEST_METHOD - the method the script should use to process the request
	5 SCRIPT_NAME - a URI path (not URL-encoded) which identifies the CGI script rather than its output. Contains no PATH_INFO segment
	6 SERVER_NAME - the name of the server host the request is directed to. Case-insensitive hostname or network address, no port
	7 SERVER_PORT - the TCP/IP port the request is received on. MUST be set even if it is the default port for the scheme
	8 SERVER_PROTOCOL - name and version of the application protocol used for this request. HTTP/1.0
	9 SERVER_SOFTWARE - name and version of the server software making the CGI request

RFC -> should
	10 REMOTE_HOST - if the hostname is not available for performance reasons or otherwise, the server MAY substitute the REMOTE_ADDR value.
	11 HTTP_*  - values read from the client request header fields. Field name is upper-cased, "-" replaced with "_", "HTTP_" prepended. Multiple fields with the same name MUST be rewritten as a single value

RFC -> "must if"
	12 CONTENT_LENGTH - the size of the message-body attached to the request, if any, in decimal number of octets.  If no data is attached, then NULL (or unset).
	13 CONTENT_TYPE - if the request includes a message-body -> set to the Internet Media Type [6] of the message-body. There is no default value for this variable.
	14 PATH_INFO - extra path (not URL-encoded) that follows SCRIPT_NAME in the request URI. /cgi-bin/hello.py/extra/data -> /extra/data. Set only if the URI has such a path
*/

#pragma once

#include "http/request/request.hpp"
#include "http/response/response.hpp"
#include "config/parser/parser.hpp"
#include "io/connection/connection_info.hpp"
#include "cgi/info.hpp"

#include <map>
#include <string>
#include <vector>

struct CgiEnv {
	CgiEnv() = delete;
	CgiEnv(const ConnInfo& info, const HttpRequest& req, const CgiInfo& cgi_info, const ServerConfig&, const std::string&);
	~CgiEnv() = default;

	// copy
	CgiEnv(const CgiEnv& other) = delete;
	CgiEnv& operator=(const CgiEnv& other) = delete;

	// move
	CgiEnv(CgiEnv&& other) = delete;
	CgiEnv& operator=(CgiEnv&& other) = delete;

	static constexpr const char*	kGatewayInterface = "CGI/1.1";
	static constexpr const char*	kServerSoftware = "webserv";

	std::vector<std::string>	env;

	// methods
	void	AddToEnv(const std::string& key, const std::string& value);
	void	AddHttpHeaders(const std::map<std::string, std::string>& headers);

	std::vector<char*>			BuildEnvCharP();
};

