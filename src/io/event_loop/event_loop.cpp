#include "utils/logger/logger.hpp"
#include "io/event_loop/event_loop.hpp"
#include "io/event_loop/signal_handler.hpp"
#include "io/socket/socket.hpp"
#include "io/instruction.hpp"
#include "utils/posix/posix.hpp"



#include <cerrno>
#include <cstddef>
#include <sys/poll.h>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
// #include <netinet/in.h>

/* ========================================================================== */
/*                             Anonymous Namespace                            */
/* ========================================================================== */

namespace {
	std::string AddrToStr(const in_addr_t& addr){
		uint32_t ip = ntohl(addr);
		std::string	str = std::to_string(ip >> 24 & 0xFF) + '.'
						+ std::to_string(ip >> 16 & 0xFF) + '.'
						+ std::to_string(ip >> 8 & 0xFF) + '.'
						+ std::to_string(ip & 0xFF);
		return str;
	}

	std::string	ResolveServerAddr(int fd, const std::string& fallback){
		sockaddr_in	local{};
		socklen_t	local_len = sizeof(local);

		if (::getsockname(fd, reinterpret_cast<sockaddr*>(&local), &local_len)){
			LOG_WARN("getsockname failed, using configured host");
			return fallback;
		}
		return AddrToStr(local.sin_addr.s_addr);
	}

	[[maybe_unused]] std::string ReventsToString(short revents) {
		std::string out;

		if (revents & POLLIN)
			out += "POLLIN|";
		if (revents & POLLOUT)
			out += "POLLOUT|";
		if (revents & POLLHUP)
			out += "POLLHUP|";
		if (revents & POLLERR)
			out += "POLLERR|";
		if (revents & POLLNVAL)
			out += "POLLNVAL|";
		if (out.empty())
			return "0";
		out.pop_back(); // remove trailing '|'
		return out;
	}

	struct	PendingInstructions{
		int				owner_fd;
		InstructionList	instructions;
	};
}

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

/* ========================================================================== */
/*                               Public Methods                               */
/* ========================================================================== */

void	EventLoop::run(){
	while (Signal::g_signal_running){
		int ready_count = pm_.Poll(kPollTimeoutMs);
		
		reaper_.CheckChildren();
		CheckCgiTimeouts();

		// early return for readability & branch prediction make it cheap
		if (ready_count < 0 /*&& !timeout?*/){
			if (HandlePollError() == Severity::Critical) // just mock, needs check
				break;
			continue;
		}
		if (ready_count == 0 /*&& !timeout*/)
			continue;

		// happy path logic
		HandleWatched(ready_count);
	}
	CleanupCgiBeforeShutdown();
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

	const Server& l = listeners_[i];

	// early return
	if (poll_entry.revents & (POLLHUP | POLLERR | POLLNVAL)){
		std::string prefix;
		if (poll_entry.revents & POLLNVAL)
			prefix = "Listener fd invalid (bug: fd closed but still in poll set): ";
		else
			prefix = "Listener failure: ";
		LOG_ERROR(prefix + " poll_fd=" + std::to_string(poll_entry.fd), l.srv_id());
		RequestShutdown();// TODO: add ADR immediate shutdown + maybe later add drain mode
		return ;
	}

	// Happy path
	AcceptConnection(poll_entry.fd, l);
}

void	EventLoop::AcceptConnection(int entry_fd, const Server& l){
	sockaddr_in		addr{};
	socklen_t		addr_len = sizeof(addr);

	int accepted_fd = ::accept(entry_fd, reinterpret_cast<sockaddr*>(&addr), &addr_len);

	// https://man7.org/linux/man-pages/man2/accept.2.html
	if (accepted_fd < 0){
		if (errno == EMFILE || errno == ENFILE){
			LOG_WARN("fd limit reached, cannot accept new connections, active: " +
						std::to_string(connections_.size()), l.srv_id());
		}
		return ;
	}
	
	Socket	accepted_socket = Socket::adopt(accepted_fd);
	if (accepted_socket.fd() < 0){
		LOG_WARN("Failed to adopt accepted client fd", l.srv_id());
		return ;// fd was closed in adopt() in case of fail
	}

	int fd = accepted_socket.fd();
	ConnInfo info{AddrToStr(addr.sin_addr.s_addr),
					ResolveServerAddr(fd, l.server_host()),
					std::to_string(l.server_port())
				};
	connections_.emplace(fd, 
				Connection(std::move(accepted_socket), info, l.srv_id(), l.server_config()));
	LOG_DEBUG("accept: new client fd=" + std::to_string(fd)
				+ " from " + info.addr,
				l.srv_id());
	pm_.Watch(fd, POLLIN);
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
	if (entry.fd != owner_fd){
		LOG_DEBUG("We are going to OnCgi fd=" + std::to_string(owner_fd)
					+ " pipe_fd=" + std::to_string(entry.fd)
					+ " revents=" + ReventsToString(entry.revents),
					connection.srv_id());
		instructions = connection.OnCgi();
	}
	else if (entry.revents & POLLIN){
		LOG_DEBUG("We are going to OnReadable fd=" + std::to_string(entry.fd)
					+ " revents=" + ReventsToString(entry.revents),
					connection.srv_id());
		instructions = connection.OnReadable();
	}
	else if (entry.revents & POLLOUT){
		LOG_DEBUG("We are going to OnWritable fd=" + std::to_string(entry.fd)
					+ " revents=" + ReventsToString(entry.revents),
					connection.srv_id());
		instructions = connection.OnWritable();
	}
	else {
		LOG_DEBUG("hangup/error on fd=" + std::to_string(entry.fd)
					+ " revents=" + ReventsToString(entry.revents),
					connection.srv_id());
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
		int		fd = instructions.list[i].target_fd;
		pid_t	pid = instructions.list[i].pid;

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
			case Action::ReapChild:
				reaper_.Track(pid);
				break;
			case Action::KillChild:
				reaper_.Abort(pid);
				break;
		}
	}
}

void	EventLoop::CloseConnection(int fd){
	auto it = connections_.find(fd);
	if (it == connections_.end())
		return; // Already closed
	
	Connection&	conn = it->second;
	const std::string ctx = it->second.srv_id();

	if (conn.InStateCgi()){
		const int	pipe_fd = conn.cgi_read_fd();
		const pid_t	pid = conn.cgi_pid();

		pm_.Unwatch(pipe_fd);
		cgi_owners_.erase(pipe_fd);
		// TODO move this to posix namespace
		if (pipe_fd != posix::kInvalidFd){
			::close(pipe_fd);
			// pipe_fd = posix::kInvalidFd;
		}
		reaper_.Abort(pid);
		LOG_DEBUG("cgi aborted process pid=" + std::to_string(pid)
					+ " fd=" + std::to_string(fd),
					ctx);
	}
	pm_.Unwatch(fd);
	connections_.erase(it);
	LOG_DEBUG("closed connection fd=" + std::to_string(fd), ctx);
}

void	EventLoop::CleanupCgiBeforeShutdown(){
	for (const auto& [fd, conn] : connections_){
		if (conn.InStateCgi())
			reaper_.Track(conn.cgi_pid());
	}
	reaper_.KillAndWaitAll();
}

void	EventLoop::CheckCgiTimeouts(){
	const Clock::time_point				now = Clock::now();
	std::vector<PendingInstructions>	to_apply;

	for (auto& [fd, conn] : connections_){
		InstructionList	instructions = conn.HandleCgiDeadline(now);
		if (instructions.count > 0)
			to_apply.push_back({fd, instructions});
	}

	for (const auto& item : to_apply){
		ApplyInstructions(item.instructions, item.owner_fd);
	}
}
