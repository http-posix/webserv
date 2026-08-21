#include "event_loop/poll_manager.hpp"
#include "logger/logger.hpp"

/* ========================================================================== */
/*                             Anonymous Namespace                            */
/* ========================================================================== */

	// struct pollfd {
	// 	int   fd;         /* file descriptor */
	// 	short events;     /* requested events */
	// 	short revents;    /* returned events */
	// };

namespace {

	struct pollfd FillPollfdStruct(int fd, short events){
		struct pollfd	tmp_pollfd;
		tmp_pollfd.fd = fd;
		tmp_pollfd.events = events;
		tmp_pollfd.revents = 0;
		return tmp_pollfd;
	}
}

/* ========================================================================== */
/*                          Constructors & Destructors                        */
/* ========================================================================== */

// sequential memory reservation to avoid resizing
// value of kInitialCapacity needs to be revisit when EventLoop starts running
// 	poll_fds_.reserve(1024);  Check exception types + 
// 	If an exception is thrown, this function has no effect 
// (strong exception guarantee)
PollManager::PollManager(){
	poll_fds_.reserve(kInitialCapacity);
}

/* ========================================================================== */
/*                               Public Methods                               */
/* ========================================================================== */

/**
 * Registers fd for watching with the given events mask.
 * @note Calling Watch() twice with the same fd creates two separate
 * entries — no deduplication. Avoiding duplicates is the caller's
 * responsibility (EventLoop), not enforced here.
 */
void PollManager::Watch(int fd, short events){
	poll_fds_.push_back(FillPollfdStruct(fd, events));
}

// 
/**
 * O(1) removal: swap with last element and shrink vector to avoid shifting
 * But this is does not preserve the order of remaining entries.
 * @note No-op (with a warning logged) if fd is not currently watched —
 * calling Unwatch() on an unknown fd is not treated as an error.
 */
void PollManager::Unwatch(int fd){
	std::vector<pollfd>::iterator it = Find(fd);

	if (it != poll_fds_.end()){
		*it = poll_fds_.back();
		poll_fds_.pop_back();
	}
	else
		LOG_WARN("fd not found: " + std::to_string(fd));
}

void PollManager::SetEvents(int fd, short events){
	std::vector<pollfd>::iterator it = Find(fd);

	if (it != poll_fds_.end()){
		it->events = events;
	}
	else
		LOG_WARN("fd not found: " + std::to_string(fd));
}


// poll() use type nfds_t (no greater than the width of long) to support 
// one or more programming environments
/**
 * Thin wrapper over the POSIX poll() system call.
 * @note Return value follows poll()'s own convention: 0 means the
 * timeout expired with no events, a positive number is the count of
 * fds with pending events (check revents to see which), and -1
 * signals an error (errno is set). PollManager does not interpret
 * this value — that's the caller's (EventLoop's) job.
 */
int PollManager::Poll(int timeout_ms){
	return ::poll(poll_fds_.data(), poll_fds_.size(), timeout_ms);
}

/* ========================================================================== */
/*                             Accessors & Mutators                           */
/* ========================================================================== */

const std::vector<pollfd>&	PollManager::poll_fds() const noexcept{
	return poll_fds_;
}

/* ========================================================================== */
/*                              Private Methods                               */
/* ========================================================================== */

// auto = std::vector<pollfd>::iterator
std::vector<pollfd>::iterator PollManager::Find(int fd){
	for (auto it = poll_fds_.begin(); it != poll_fds_.end(); ++it){
		if (it->fd == fd)
			return it;
	}
	return poll_fds_.end();
}
