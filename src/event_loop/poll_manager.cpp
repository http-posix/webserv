#include "event_loop/poll_manager.hpp"
#include "logger/logger.hpp"

/* ========================================================================== */
/*                             Anonymous Namespace                            */
/* ========================================================================== */

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
PollManager::PollManager(){
	poll_fds_.reserve(kInitialCapacity);
}

/* ========================================================================== */
/*                               Public Methods                               */
/* ========================================================================== */

// filling struct with actual fd and events
// Watch() twice on the same fd creates two separate entries —
// caller's responsibility to avoid this
void PollManager::Watch(int fd, short events){
	poll_fds_.push_back(FillPollfdStruct(fd, events));
}

// swap and delete from end
// last element become *it and then we delete last element
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

