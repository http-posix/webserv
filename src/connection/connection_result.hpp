#pragma once

#include <vector>

// This is a contract between EventLoop & Connection
// EventLoop has the right to manage fds related to Connection,
//  but it knows nothing about the Connection current state

// Connection knows about it's state and can give back instruction on
//  what should be done with fd according to that state

enum class Action {
	WatchReadable,
	WatchWritable,
	StopWatching,
	CloseConnection
};

struct Instruction {
	Action	action;
	int		target_fd;
};

using InstructionList std::vector<Instruction> instructions;
// ConnectionResult => InstructionList