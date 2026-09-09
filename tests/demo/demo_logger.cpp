#include "utils/logger/logger.hpp"
#include "utils/logger/colors.hpp"

#include <iostream>

int main(){
	std::cout << clrs::kMagenta << "=== Logger demo ===\n\n" << clrs::kReset;
	Logger::PrintMsg("Log levels for Webserv with context:");

	LOG_DEBUG("Sample message");
	LOG_INFO("Sample message");
	LOG_WARN("Sample message");
	LOG_ERROR("Sample message");

	Logger::PrintMsg("\nLog levels for Webserv with server context:");
	const char* ctx = "[127.0.0.1:8080 fd=4]";
	LOG_DEBUG("Sample message", ctx);
	LOG_INFO("Sample message", ctx);
	LOG_WARN("Sample message", ctx);
	LOG_ERROR("Sample message", ctx);
	return 0;
}
