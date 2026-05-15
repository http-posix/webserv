#pragma once

// !!! This works if destructor deletes dynamic resources - RAII
// Types we probably will have: Config, Socket, Runtime ?
// What about Parse exceptions - only bad_alloc? there only theoretically possible bad_alloc ?
// Only for fatal error
// Open-Closed Principle - sOlid

// Is it eligible I use errno just to show correct error msg not to handle server behavior ?

// Explicit keyword used for safety so we can't accidentally pass string as parameter 

#include <stdexcept>
#include <string>

class AppException : public std::runtime_error{
	public:
		explicit AppException(const std::string& error_msg) :
			std::runtime_error(error_msg)
			{	};
};

// Using keyword  

class ConfigException : public AppException {
	public:
		using AppException::AppException;
};

// poll() socket() listen() bind()
class ServerException : public AppException{
	public:
		using AppException::AppException;
};

class CgiException : public AppException {
	public:
		using AppException::AppException;
};
