#include "logger/logger.hpp"
#include "event_loop/event_loop.hpp"
#include "event_loop/signal_handler.hpp"
#include "socket/socket.hpp"
#include "connection/instruction.hpp"

#include <cerrno>
#include <cstddef>
#include <sys/poll.h>
#include <sys/socket.h>
#include <cstring>

/* 
POLLIN
The file descriptor is ready for reading data other than high-priority data.
POLLRDNORM
The file descriptor is ready for reading normal data.
POLLRDBAND
The file descriptor is ready for reading priority data.
POLLPRI
The file descriptor is ready for reading high-priority data.
POLLOUT
The file descriptor is ready for writing normal data.
POLLWRNORM
Equivalent to POLLOUT.
POLLWRBAND
The file descriptor is ready for writing priority data.
POLLERR
An error condition is present on the file descriptor. All error conditions that arise solely from the state of the object underlying the open file description and would be diagnosed by a return of -1 from a read() or write() call on the file descriptor shall be reported as a POLLERR event. This flag is only valid in the revents bitmask; it shall be ignored in the events member.
POLLHUP
A device has been disconnected, or a pipe or FIFO has been closed by the last process that had it open for writing. Once set, the hangup state of a FIFO shall persist until some process opens the FIFO for writing or until all read-only file descriptors for the FIFO are closed. This event and POLLOUT are mutually-exclusive. However, this event and POLLIN, POLLRDNORM, POLLRDBAND, or POLLPRI are not mutually-exclusive. This flag is only valid in the revents bitmask; it shall be ignored in the events member.
POLLNVAL
The specified fd value is not an open file descriptor. This flag is only valid in the revents member; it shall be ignored in the events member.
*/


/* ========================================================================== */
/*                          Constructors & Destructors                        */
/* ========================================================================== */

EventLoop::EventLoop(std::vector<Server>&& listeners):
	listeners_(std::move(listeners)),
	size_listeners_(listeners_.size())
{
	for (size_t i = 0; i < size_listeners_; i++){
		pm_.Watch(listeners_[i].fd(), POLLIN);
	}
}



/*
ERRORS
The poll() and ppoll() functions shall fail if:

[EAGAIN]
The allocation of internal data structures failed but a subsequent request may succeed.
[EINTR]
A signal was caught during poll() or ppoll().
[EINVAL]
The nfds argument is greater than {OPEN_MAX}.
The ppoll() function shall fail if:

[EINVAL]
An invalid timeout interval was specified.
*/

/* ========================================================================== */
/*                               Public Methods                               */
/* ========================================================================== */

void	EventLoop::run(){
	while (Signal::g_signal_running){
		int ready_count = pm_.Poll(kPollTimeoutMs);
		
		// check timeout once here, because if I'll do it in the end I'll never get there if I'll in if-continue loop

		// early return for readability & branch prediction make it cheap
		if (ready_count < 0 /*&& !timeout?*/){
			if (HandlePollError() == Severity::Critical) // просто заглушка, треба рефактор
				break;
			continue;
		}
		if (ready_count == 0 /*&& !timeout*/)
			continue;

		// happy path logic
		HandleWatched(ready_count);
	}
}

/* ========================================================================== */
/*                              Private Methods                               */
/* ========================================================================== */

EventLoop::Severity	EventLoop::HandlePollError(){
	if (errno == EAGAIN || errno == EINTR) // Resource temporarily unavailable || signal catched
		return Severity::NonCritical;
	if (errno == EINVAL){
		LOG_ERROR("poll() failed due timeout: " + std::string(strerror(errno)));
		return Severity::Critical;
	}
	// to handle amount of poll_fds_ I can set my own restriction
	// like MAX_POLLFDS = 1024 and then 2 strategy 
	// => accept() check_max_pollfds => close() 
	//  or shutdown evenloop
	// ====> set fd to wait -> need to investigate this approach

	return Severity::NonCritical; // ?????
}

void	EventLoop::HandleWatched(int ready_count){
	const std::vector<pollfd>	watched_fds= pm_.poll_fds();
	int							processed_count = 0;

	for (size_t i = 0; i < watched_fds.size()
						&& processed_count < ready_count
						&& Signal::g_signal_running; i++){
		// POD - Plain Old Data - no reason to make a reference, by value would be faster
		// 8 byte
		const pollfd	entry = watched_fds[i];

		// early continue - guard clauses
		if (entry.revents == 0)
			continue;

		// i < watched_fds.size() is invariant for listeners
		if (i < size_listeners_)
			HandleListener(entry, i);
		else
			HandleConnectionEvent(entry);
		processed_count++;
	}
}


/*int accept(int socket, struct sockaddr *restrict address,
       socklen_t *restrict address_len);*/

void	EventLoop::HandleListener(const pollfd poll_entry, size_t i){
	// Contract 
	assert(i < size_listeners_);
	assert(poll_entry.fd == listeners_[i].fd());

	// early return
	if (poll_entry.revents & (POLLHUP | POLLERR | POLLNVAL)){
		std::string prefix;
		if (poll_entry.revents & POLLNVAL)
			prefix = "Listener fd invalid (bug: fd closed but still in poll set), fd: ";
		else
			prefix = "Listener failure, fd: ";
		LOG_ERROR(prefix + std::to_string(poll_entry.fd) +
			" Host: " + listeners_[i].server_host() +
			" Port: " + std::to_string(listeners_[i].server_port()));
		RequestShutdown();// TODO: add ADR immediate shutdown + maybe later add drain mode
		return ;
	}

	// Happy path
	int accepted_fd = AcceptConnection(poll_entry.fd);

	// https://man7.org/linux/man-pages/man2/accept.2.html
	if (accepted_fd < 0){
		if (errno == EMFILE || errno == ENFILE){
			LOG_WARN("fd limit reached, cannot accept new connections, active: " +
						std::to_string(connections_.size()));
		}
		return ;
	}
	Socket	accepted_socket = Socket::adopt(accepted_fd);
	if (accepted_socket.fd() < 0){
		return ;// fd was closed in adopt() in case of fail
	}
	// HttpParser should have move ctor too

	// To avoid loosing data 'cause of unspecified 
	// order of evaluation of function arguments
	int fd = accepted_socket.fd();
	connections_.emplace(fd, Connection(std::move(accepted_socket)));
	pm_.Watch(fd, POLLIN);
}

// nullptr, nullptr - if I don't want to keep info about IP, but I want
// type punning via a common prefix
// prefix-based type punning
// common-prefix type punning
// for IPv4 & IPv6 struct starts from the same field sa_family_t sa_family
// https://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/socket.h.html
int	EventLoop::AcceptConnection(int listener_fd){
	sockaddr_storage	addr{};
	socklen_t			addr_len = sizeof(addr);

	int fd = ::accept(listener_fd, reinterpret_cast<sockaddr*>(&addr), &addr_len);
	return fd;
}

void	EventLoop::HandleConnectionEvent(const pollfd entry){
	// Find owner - is this CGI connection or normal one
	int owner_fd = FindOwner(entry.fd);

	// Find Connection instance
	auto it = connections_.find(owner_fd);
	if (it == connections_.end()) // Connection has been already closed
		return;
	
	Connection&		connection = it->second;
	InstructionList	instructions;

	// First resolve is it cgi or not, then check what happened
	if (entry.fd != owner_fd)
		instructions = connection.OnCgi();
	else if (entry.revents & POLLIN)
		instructions = connection.OnReadable();
	else if (entry.revents & POLLOUT)
		instructions = connection.OnWritable();
	else {
		// TODO: proper POLLERR/POLLHUP/POLLNVAL handling (different for cgi pipe vs client fd)
		CloseConnection(owner_fd);
		return ;
	}

	ApplyInstructions(instructions, owner_fd);
}

void	EventLoop::RequestShutdown(){
	Signal::g_signal_running = 0;
}

int	EventLoop::FindOwner(int fd){
	auto it = cgi_owners_.find(fd);
	if (it == cgi_owners_.end()) // Early return
		return fd;
	return it->second;
}

/* in this case it will leave only inside if statement
if (auto it = cgi_owners_.find(fd); it != cgi_owners_.end())
	return it->second;
return fd;
*/


void	EventLoop::ApplyInstructions(const InstructionList& instructions, int owner_fd){
	// At this moment don't know is cgi or not
	// There could be queue of instructions if this is cgi
	// For common connection we expected only 1 instruction
	for (int i = 0; i < instructions.count; i++){
		int fd = instructions.list[i].target_fd;
		switch (instructions.list[i].action){
			case Action::WaitReadable:
				pm_.SetEvents(fd, POLLIN);
				break;
			case Action::WaitWritable:
				pm_.SetEvents(fd, POLLOUT);
				break;
			case Action::WatchCgi:
				pm_.Watch(fd, POLLIN);
				cgi_owners_[fd] = owner_fd;
				break;
			case Action::StopWatching:
				pm_.Unwatch(fd);
				cgi_owners_.erase(fd);
				break;
			case Action::CloseConnection:
				CloseConnection(owner_fd); // Check logic when CGI will work
				return;
		}
	}
}

void	EventLoop::CloseConnection(int fd){
	auto it = connections_.find(fd);
	if (it == connections_.end())
		return; // Already closed
	
		// CGI ?
	pm_.Unwatch(fd);
	connections_.erase(it);
	LOG_DEBUG("Close connection " + std::to_string(fd));
}
