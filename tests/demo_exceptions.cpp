#include "app_exception/app_exception.hpp"
#include <iostream>
#include <cassert>

int main() {
	std::cout << "=== AppException demo ===\n\n";

	// test 1
	bool caught = false;
	try { throw ConfigException("config error"); }
	catch (const AppException& e) {
		caught = true;
		assert(std::string(e.what()) == "config error");
	}
	assert(caught);
	std::cout << "ConfigException caught by AppException [OK]\n";

	// test 2
	caught = false;
	try { throw ServerException("server error"); }
	catch (const AppException& e) {
		caught = true;
		assert(std::string(e.what()) == "server error");
	}
	assert(caught);
	std::cout << "ServerException caught by AppException [OK]\n";

	// test 3
	caught = false;
	try { throw CgiException("cgi error"); }
	catch (const AppException& e) {
		caught = true;
		assert(std::string(e.what()) == "cgi error");
	}
	assert(caught);
	std::cout << "CgiException caught by AppException [OK]\n";

	return 0;
}