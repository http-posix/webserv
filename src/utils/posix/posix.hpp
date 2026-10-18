#pragma once

#include <sys/types.h>
#include <string>

namespace posix {
	inline constexpr int	kInvalidFd = -1;
	inline constexpr pid_t	kInvalidPid = -1;
	inline constexpr pid_t	kForkChild = 0;
	inline constexpr int	kSyscallError = -1;
	inline constexpr int	kSuccess = 0;

	bool	SetNonBlockingMode(int fd, const std::string& fd_type, const std::string& ctx="");
}
