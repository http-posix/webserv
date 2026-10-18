#include "utils/posix/posix.hpp"
#include "utils/logger/logger.hpp"

#include <fcntl.h>
#include <cerrno>
#include <cstring>

bool	posix::SetNonBlockingMode(int fd, const std::string& fd_type, const std::string& ctx){
// read-modify-write pattern is a best practice in POSIX
// but flag F_GETFL isn't allowed in this project
// in this particular project this is ok, 'cause we set flags on just created socket wich hasn't any other flags
/* 
	int flags = ::fcntl(fd, F_GETFL, 0);
	if (flags == posix::kSyscallError)
	{
		LOG_ERROR("fcntl(F_GETFL) failed on" + fd_type + " " + std::to_string(fd) + ": " + std::string(strerror(errno)));
		return false;
	}
*/
	if (::fcntl(fd, F_SETFL, O_NONBLOCK) == posix::kSyscallError){
		LOG_ERROR("fcntl(F_SETFL) failed on " + fd_type + " "
			+ std::to_string(fd) + ": "
			+ std::string(strerror(errno))
			+ ctx);
		return false;
	}
	return true;
}
