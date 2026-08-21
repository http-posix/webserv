#pragma once

#include "server/server.hpp"
#include <cstddef>
#include "event_loop/poll_manager.hpp"
#include "connection/connection.hpp"


/// need to give locations from config to response

class EventLoop{
	public:
		EventLoop() = delete;
		explicit EventLoop(std::vector<Server> listeners);
		~EventLoop() = default;

		void run();

	private:
		std::vector<Server>					listeners_;
		// not kVariableName `cause value defined in runtime. not a compile time
		const size_t						size_listeners_;
		static int							timeout_; //60-75 sec in NGNIX – we need ms(!)
		PollManager							pm_;
		std::unordered_map<int, Connection>	connections_;
		std::unordered_map<int, int>		cgi_owners_;

		enum class Severity{
			NonCritical,
			Critical
		};

		// methods
		Severity	HandlePollError();
		void		HandleWatched(int ready_count);//finished
		void		HandleListener(pollfd poll_entry, size_t i);//still need to check error behavior
		void		HandleConnectionEvent(pollfd poll_entry);
		int			HandleCGI();
		int			AcceptConnection(int listener);
		void		RequestShutdown();
		void		CloseConnection(int fd);
};

