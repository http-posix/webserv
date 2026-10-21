#include "utils/posix/posix.hpp"
#include "utils/logger/logger.hpp"

#include <fcntl.h>
#include <cerrno>
#include <cstring>

/* ========================================================================== */
/*                               Public Functions                             */
/* ========================================================================== */

// read-modify-write pattern is a best practice in POSIX
// but flag F_GETFL isn't allowed in this project
// in this particular project this is ok,
// because we set flags on just created fd which hasn't any other flags
bool	posix::SetNonBlockingMode(int fd, const std::string& fd_type, const std::string& ctx){
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
