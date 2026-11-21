ServerRoot			/usr/local/etc/httpd  
ScriptAlias			/cgi-bin/  
AddType       application/x-httpd-cgi          .pl   .sh   .cgi  

https://www.rfc-editor.org/info/rfc3875/#section-4

Robinson & Coar              Informational                     [Page 10]

RFC 3875                    CGI Version 1.1                 October 2004


      meta-variable-name = "AUTH_TYPE" | "CONTENT_LENGTH" |
                           "CONTENT_TYPE" | "GATEWAY_INTERFACE" |
                           "PATH_INFO" | "PATH_TRANSLATED" |
                           "QUERY_STRING" | "REMOTE_ADDR" |
                           "REMOTE_HOST" | "REMOTE_IDENT" |
                           "REMOTE_USER" | "REQUEST_METHOD" |
                           "SCRIPT_NAME" | "SERVER_NAME" |
                           "SERVER_PORT" | "SERVER_PROTOCOL" |
                           "SERVER_SOFTWARE" | scheme |
                           protocol-var-name | extension-var-name
      protocol-var-name  = ( protocol | scheme ) "_" var-name
      scheme             = alpha *( alpha | digit | "+" | "-" | "." )
      var-name           = token
      extension-var-name = token

This specification does not distinguish between zero-length (NULL)
   values and missing values.  For example, a script cannot distinguish
   between the two requests http://host/script and http://host/script?
   as in both cases the QUERY_STRING meta-variable would be NULL.


	
 Yes: CONTENT_LENGTH, CONTENT_TYPE, GATEWAY_INTERFACE, PATH_INFO

 No: AUTH_TYPE


Prototype:
```
location /cgi-bin {
    root /var/www/cgi;
    allowed_methods GET POST;
    cgi .py  /usr/bin/python3;
}
```

    env = [
        "GATEWAY_INTERFACE=CGI/1.1",
        "SERVER_PROTOCOL=HTTP/1.1",
        "REQUEST_METHOD="  + request.method,
        "SCRIPT_FILENAME=" + script_path,
        "SCRIPT_NAME="     + path part of request.path,
        "PATH_INFO="       + path part of request.path,
        "QUERY_STRING="    + part after '?' (or empty),
        "SERVER_NAME="     + server host,
        "SERVER_PORT="     + server port,
        "REDIRECT_STATUS=200"                // php-cgi refuses to run without it
    ]


# Infor draft from first code

```cpp
#pragma once

#include <string>

struct CgiInfo
{
	std::string	interpreter;	// "/usr/bin/python3"		— location's cgi rule for this extension
	std::string	script_name;	// "/cgi-bin/hello.py"		— request path up to the end of the script
	std::string	path_info;		// "/extra/path" or ""		— request path after script_name
	std::string	query_string;	// "a=1&b=2" or ""			— part of the URI after '?'
	std::string	script_filename;// "./www/cgi-bin/hello.py"	 — script_name mapped through location root
};

/*
	CgiRequest					cgi_info;

	cgi_info.interpreter = "/usr/bin/python3";
	cgi_info.script_name = "/cgi-bin/hello.py";
	cgi_info.path_info = ""; // need to be implemented
	cgi_info.query_string = ""; //
	cgi_info.script_filename = "./www/cgi-bin/hello.py";
	// request_body	already in req.body_
*/
```