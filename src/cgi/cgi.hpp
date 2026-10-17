#pragma once

#include "cgi/env.hpp"
#include "cgi/info.hpp"
#include "http/http_status_code.hpp"


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
		HttpStatusCode			SpawnChild();
		const pid_t&					process_num() const noexcept;
		const std::array<int, 2>&		pipe_fd() const noexcept;

	private:
		constexpr static int	kInvalidValue = -1;
		constexpr static int	kChildProc = 0;
		constexpr static int	kMaxFd = 1024;

		std::array<int, 2>		pipe_fd_ = {kInvalidValue, kInvalidValue};
		pid_t					pid_ = kInvalidValue;

		std::string		srv_id_;
		CgiEnv			cgi_env_;
		std::string		interpreter_;// "/usr/bin/python3" for execve() and argv[0]

		std::string		script_dir_;// "./www/cgi-bin" for chdir()
		std::string		script_file_;// "hello.py" for argv[1] after chdir()
		char*			argv_[3] ={};

		//methods
		void	BuildArgv(const std::string& script_filename);
		bool	SetPipeFd(int& fd);
		void	ClosePipeFds();
		void	CloseFd(int& fd);
		void	CloseInheritedFds();
	};
