#include "cgi.hpp"

/*
env variables

RFC -> must

GATEWAY_INTERFACE - the dialect of CGI used by the server. CGI/1.1
QUERY_STRING - URL-encoded search or parameter string. If the Script-URI has no query component, MUST be defined as an empty string ""
REMOTE_ADDR - the network address of the client sending the request
REQUEST_METHOD - the method the script should use to process the request
SCRIPT_NAME - a URI path (not URL-encoded) which identifies the CGI script rather than its output. Contains no PATH_INFO segment
SERVER_NAME - the name of the server host the request is directed to. Case-insensitive hostname or network address, no port
SERVER_PORT - the TCP/IP port the request is received on. MUST be set even if it is the default port for the scheme
SERVER_PROTOCOL - name and version of the application protocol used for this request. HTTP/1.0
SERVER_SOFTWARE - name and version of the server software making the CGI request

RFC -> should

REMOTE_HOST - if the hostname is not available for performance reasons or otherwise, the server MAY substitute the REMOTE_ADDR value.
HTTP_*  - values read from the client request header fields. Field name is upper-cased, "-" replaced with "_", "HTTP_" prepended. Multiple fields with the same name MUST be rewritten as a single value

RFC -> "must if"
CONTENT_LENGTH - the size of the message-body attached to the request, if any, in decimal number of octets.  If no data is attached, then NULL (or unset).
CONTENT_TYPE - if the request includes a message-body -> set to the Internet Media Type [6] of the message-body. There is no default value for this variable. 
*/

#pragma once

sruct CgiEnv {
	
};