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