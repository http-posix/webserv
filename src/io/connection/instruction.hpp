#pragma once

#include <array>
#include <cassert>
// This is a contract between EventLoop & Connection
// EventLoop has the right to manage fds related to Connection,
//  but it knows nothing about the Connection current state

// Connection knows about it's state and can give back instruction on
//  what should be done with fd according to that state

enum class Action {
	WaitReadable, // For keep_alive, after OnWritable() is finished
	WaitWritable, // After parser return Complete
	WatchCgi, // Add pipe fd to PollManager pm_
	StopWatching, // CGI finished + Connection is keep_alive
	CloseConnection // Timeout or error from Connection, notCGI
};
// additional states for CGI

struct Instruction {
	Action	action;
	int		target_fd;
};

struct InstructionList{
	static constexpr int						kMaxInstructions = 3;
	std::array<Instruction, kMaxInstructions>	list;
	int											count = 0;

	void Add(Action action, int target_fd){
		assert(count < kMaxInstructions);
		list[count] = {action, target_fd};
		count++;
	}
};
