#pragma once

#include "cgi/env.hpp"
#include "cgi/info.hpp"
#include "http/http_status_code.hpp"
#include "utils/posix/posix.hpp"

#include <string>
#include <sys/types.h>
#include <array>


class Cgi{
	public:
		Cgi() = delete;
		Cgi(const ConnInfo& info, const HttpRequest& req, const CgiInfo& cgi_info, const ServerConfig& cfg, const std::string& srv_id);
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

	private:
		constexpr static int	kAssumedMaxFds = 1024;
		static constexpr int	kReadEnd = 0;
		static constexpr int	kWriteEnd = 1;

		std::array<int, 2>		pipe_fds_ = {posix::kInvalidFd, posix::kInvalidFd};
		pid_t					pid_ = posix::kInvalidPid;

		std::string		srv_id_;
		CgiEnv			cgi_env_;
		std::string		interpreter_;// "/usr/bin/python3" for execve() and argv[0]

		std::string		script_dir_;// "./www/cgi-bin" for chdir()
		std::string		script_file_;// "hello.py" for argv[1] after chdir()
		char*			argv_[3] ={};

		//methods
		void	BuildArgv(const std::string& script_filename);
		void	ClosePipeFds();
		void	CloseFd(int& fd);
		void	CloseInheritedFds();
	};
