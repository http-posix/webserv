#include "logger/logger.hpp"
#include "logger/colors.hpp"
#include <iostream>

void inner_function(){
	std::cerr << "\n";
	Logger::PrintMsg("This is inside inner_function:");
	LOG_DEBUG("DEBUG macro tested");
	LOG_INFO("INFO macro tested");
	LOG_WARN("WARN macro tested");
	LOG_ERROR("ERROR macro tested");
}

int main(){
	std::cout << clrs::kMagenta << "=== Logger demo ===\n\n" << clrs::kReset;
	Logger::PrintMsg("Log levels for Webserv:");
	LOG_DEBUG("DEBUG macro tested");
	LOG_INFO("INFO macro tested");
	LOG_WARN("WARN macro tested");
	LOG_ERROR("ERROR macro tested");
	inner_function();
	return 0;
}
