#pragma once

#include <sys/types.h>
#include <unordered_set>

/*
 * Manage child-process with waitpid without blocking the loop
 * 
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
