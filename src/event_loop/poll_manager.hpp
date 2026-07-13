#pragma once

#include <cstddef>
#include <poll.h>
#include <vector>

/**
 * Thin wrapper over std::vector<pollfd> and the system poll().
 * Registers/unregisters fds it's told to watch — knows nothing about
 * what kind of fd it is (listener/client/cgi pipe); that's EventLoop's job.
 * Does not own any fd (never opens or closes one).
 */

class PollManager{
	public:
		PollManager();
		~PollManager() = default;

		// Copy
		PollManager(const PollManager& other) = delete;
		PollManager& operator=(const PollManager& other) = delete;

		// Move
		PollManager(PollManager&& other) noexcept = default;
		PollManager& operator=(PollManager&& other) noexcept = default;

		void Watch(int fd, short events);
		void Unwatch(int fd);
		void SetEvents(int fd, short events);

		int Poll(int timeout_ms);

		// accessor
		const std::vector<pollfd>&	poll_fds() const noexcept;
	
	private:
		static constexpr size_t			kInitialCapacity = 1024;
		std::vector<struct pollfd>		poll_fds_;

		// methods
		std::vector<pollfd>::iterator	Find(int fd);

};
