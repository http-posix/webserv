#pragma once

#include <sys/types.h>
#include <variant>

struct StateReading {}

struct StateWriting {
	size_t offset = 0;
};

struct StateCGI {
	int pipe_fd;
	pid_t pid;
};

// using here is type alias
using ConnectionState = std::variant<StateReading, StateWriting, StateCGI>;

