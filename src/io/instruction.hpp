#pragma once

#include "utils/posix/posix.hpp"

#include <array>
#include <cassert>
#include <sys/types.h>
// This is a contract between EventLoop & Connection
// EventLoop has the right to manage fds related to Connection,
//  but it knows nothing about the Connection current state

// Connection knows about it's state and can give back instruction on
//  what should be done with fd/pid according to that state

enum class Action {
	WaitReadable, // For keep_alive, after OnWritable() is finished
	WaitWritable, // After parser return Complete
	WatchCgi, // Add pipe fd to PollManager pm_
	StopWatching, // CGI finished + Connection is keep_alive
	CloseConnection, // Timeout or error from Connection, notCGI
	ReapChild, // pid is a target, child catch EOF
	KillChild // --//--  kill now, then reap (read error)
};

struct Instruction {
	Action	action;
	int		target_fd = posix::kInvalidFd;
	pid_t	pid = posix::kInvalidPid;
};

struct InstructionList{
	static constexpr int						kMaxInstructions = 3;
	std::array<Instruction, kMaxInstructions>	list;
	int											count = 0;

	void Add(Action action, int target_fd){
		assert(count < kMaxInstructions);
		assert(!IsPidAction(action));
		list[count] = {action, target_fd, posix::kInvalidPid};
		count++;
	}

	void AddPid(Action action, pid_t pid){
		assert(count < kMaxInstructions);
		assert(IsPidAction(action));
		assert(pid > 0);
		list[count] = {action, posix::kInvalidFd, pid};
		count++;
	}

	static bool IsPidAction(Action action){
		return action == Action::ReapChild || action == Action::KillChild;
	}
};
