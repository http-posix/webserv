#pragma once

#include <sys/types.h>
#include <unordered_set>

/**
 * Collects CGI child processes without blocking the loop.
 *
 * - Track(): the child is expected to exit by itself (pipe EOF).
 * - Abort(): SIGKILL now, reap later.
 * - CheckChildren(): waitpid(WNOHANG) for every tracked pid, once per tick.
 * - KillAndWaitAll(): shutdown only, blocking.
 *
 * Invariant: pending_ holds only pids that are not reaped yet.
 * A pid is erased right after waitpid(): after reaping, the system can
 * give the same pid to another process, and we must not kill it.
 */
class ChildReaper{
	public:
		ChildReaper() = default;
		~ChildReaper() = default;

		// Copy
		ChildReaper(const ChildReaper& other) = delete;
		ChildReaper& operator=(const ChildReaper& other) = delete;

		// Move
		ChildReaper(ChildReaper&& other) = delete;
		ChildReaper& operator=(ChildReaper&& other) = delete;

		void	Track(pid_t pid);
		void	Abort(pid_t pid);
		void	CheckChildren();
		void	KillAndWaitAll(); // Shutdown only

	private:
		std::unordered_set<pid_t> pending_;
};
