#pragma once

#include "cgi/env.hpp"
#include "http/http_status_code.hpp"
#include "utils/posix/posix.hpp"

#include <string>
#include <sys/types.h>
#include <array>

/**
 * Starts one CGI script as a child process.
 *
 * - Builds argv (interpreter + script file) and the RFC 3875 environment.
 * - Child: stdout -> pipe, stdin -> /dev/null, chdir to the script dir,
 *   execve(interpreter).
 * - Parent: keeps the non-blocking read end and the pid.
 *
 * Short-lived: lives only inside Connection::HandleCompleteRequest().
 * Ownership of the pipe fd and the pid moves to StateCGI.
 */
class Cgi{
	public:
		Cgi() = delete;
		Cgi(const ConnInfo& info, const HttpRequest& req, const CgiRequest& cgi_info, const std::string& srv_id);
		~Cgi() = default;

		// Copy
		Cgi(const Cgi& other) = delete;
		Cgi& operator=(const Cgi& other) = delete;

		// Move
		Cgi(Cgi&& other) = delete;
		Cgi& operator=(Cgi&& other) = delete;

		// methods
		HttpStatusCode		SpawnChild();
		pid_t				child_pid() const noexcept;
		int					pipe_read_fd() const noexcept;
		int					pipe_write_fd() const noexcept;

	private:
		static constexpr int	kAssumedMaxFds = 1024;
		static constexpr int	kReadEnd = 0;
		static constexpr int	kWriteEnd = 1;

		std::array<int, 2>		stdout_pipe_ = {posix::kInvalidFd, posix::kInvalidFd};
		pid_t					pid_ = posix::kInvalidPid;
		std::array<int, 2>		stdin_pipe_ = {posix::kInvalidFd, posix::kInvalidFd};

		std::string		srv_id_;
		CgiEnv			cgi_env_;
		std::string		interpreter_;// "/usr/bin/python3" for execve() and argv[0]
		bool			has_body_;

		std::string		script_dir_;// "./www/cgi-bin" for chdir()
		std::string		script_file_;// "hello.py" for argv[1] after chdir()
		char*			argv_[3] ={};

		//methods
		void	BuildArgv(const std::string& script_filename);
		void	ClosePipeFds(std::array<int, 2>& fds);
		void	CloseFd(int& fd);
		void	CloseInheritedFds();
		bool	OpenPipe(std::array<int, 2>& fds, int server_end, const char* name);
	};
