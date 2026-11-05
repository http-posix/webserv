#include "io/event_loop/child_reaper.hpp"
#include "utils/posix/posix.hpp"
#include "utils/logger/logger.hpp"

#include <signal.h>
#include <sys/wait.h>
#include <cerrno>
#include <string>

void	ChildReaper::Track(pid_t pid){
	if (pid > 0)
		pending_.insert(pid);
}

void	ChildReaper::Abort(pid_t pid){
	if (pid > 0){
		::kill(pid, SIGKILL); // async, only send signal - we don't know when kernel will do it
		pending_.insert(pid);
	}
}

void	ChildReaper::CheckChildren(){
	for (auto it = pending_.begin(); it != pending_.end(); ){
		const pid_t	pid = *it;
		int			status = 0;
		pid_t		ret_val = ::waitpid(pid, &status, WNOHANG);

		if (ret_val == 0){
			it++;
			continue;
		}
		it = pending_.erase(it); // return value is an iterator following the last removed element
		if (ret_val == posix::kSyscallError)
			LOG_WARN("waitpid failed pid=" + std::to_string(pid));
		else
			LOG_DEBUG("reaped child pid=" + std::to_string(pid));
	}
}

void	ChildReaper::KillAndWaitAll(){
	for (pid_t pid : pending_){
		::kill(pid, SIGKILL);
		while (waitpid(pid, nullptr, 0) == posix::kSyscallError && errno == EINTR);
	}
	pending_.clear();
}
