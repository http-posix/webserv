#pragma once

#include "utils/posix/posix.hpp"

#include <sys/types.h>
#include <variant>
#include <string>

struct StateReading {};

struct StateWriting {
	std::string	buffer;
	size_t		offset = 0;
};

struct StateCGI {
	int		pipe_read_fd = posix::kInvalidFd;
	pid_t	child_pid = posix::kInvalidPid;
};

// using here is type alias
using ConnectionState = std::variant<StateReading, StateWriting, StateCGI>;

// a type-safe union
// An instance of variant at any given time either holds a value of one of 
// its alternative types, or in the case of error - no value (this state is
// hard to achieve, see valueless_by_exception).

// https://dev.to/pauljlucas/unions-stdvariant-in-c-2op1