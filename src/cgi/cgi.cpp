#include "cgi/cgi.hpp"
#include "utils/logger/logger.hpp"
#include "io/event_loop/signal_handler.hpp"

#include <unistd.h>
#include <sys/wait.h>
#include <csignal>
#include <cstdlib>

/* ========================================================================== */
/*                             Anonymous Namespace                            */
/* ========================================================================== */

namespace {
	std::pair<std::string, std::string>	SplitPath(const std::string& path){
		size_t	slash = path.rfind('/');

		if (slash == std::string::npos)
			return {".", path};
		if (slash == 0)
			return {"/", path.substr(1)};
		return {path.substr(0, slash), path.substr(slash + 1)};
		}
}

/* ========================================================================== */
/*                          Constructors & Destructors                        */
/* ========================================================================== */

Cgi::Cgi(const ConnInfo& info,
		const HttpRequest& req,
		const CgiInfo& cgi_info,
		const ServerConfig& cfg,
		const std::string& srv_id):
	srv_id_(srv_id),
	cgi_env_(info, req, cgi_info, cfg, srv_id),
	interpreter_(cgi_info.interpreter)
{
	BuildArgv(cgi_info.script_filename);
}

/* ========================================================================== */
/*                               Public Methods                               */
/* ========================================================================== */

HttpStatusCode	Cgi::SpawnChild(){
	
	if (::pipe(pipe_fds_.data()) == posix::kSyscallError){
		LOG_ERROR("Failed pipe() ", srv_id_);
		return HttpStatusCode::InternalServerError;
	}
	pid_ = ::fork();
	if (pid_ == posix::kInvalidPid){
		LOG_ERROR("Failed fork() ", srv_id_);
		ClosePipeFds();
		return HttpStatusCode::InternalServerError;
	}
	// who check access to script - here or in http parser?
	// TODO: access(F_OK) -> 404, access(R_OK) -> 403. Do it BEFORE pipe() (here or router), otherwise missing script = 502
	if (pid_ == posix::kForkChild){
		if (dup2(pipe_fds_[1], STDOUT_FILENO) == posix::kSyscallError){
			LOG_ERROR("dup2() failed",  srv_id_);
			std::_Exit(1);
		}
		CloseInheritedFds();
		if (chdir(script_dir_.c_str()) == posix::kSyscallError)
			std::_Exit(1);
		Signal::SetupDefaultSignals();
		execve(interpreter_.c_str(), argv_, cgi_env_.Envp());
		std::_Exit(127);
	}
	CloseFd(pipe_fds_[1]);
	if (!posix::SetNonBlockingMode(pipe_fds_[0], "pipe fd", srv_id_)){
		kill(pid_, SIGKILL);
		waitpid(pid_, nullptr, 0);
		ClosePipeFds();
		return HttpStatusCode::InternalServerError;
	}
	return HttpStatusCode::OK;
}

/* ========================================================================== */
/*                             Accessors & Mutators                           */
/* ========================================================================== */

int	Cgi::pipe_read_fd() const noexcept{
	return pipe_fds_[0];
}

pid_t	Cgi::child_pid() const noexcept{
	return pid_;
}

/* ========================================================================== */
/*                              Private Methods                               */
/* ========================================================================== */

void	Cgi::BuildArgv(const std::string& script_filename){
	auto[dir, file] = SplitPath(script_filename);
	script_dir_ = dir;
	script_file_ = file;
	argv_[0] = interpreter_.data();
	argv_[1] = script_file_.data();
}

void	Cgi::CloseFd(int& fd){
	::close(fd);
	fd = posix::kInvalidFd;
}

void	Cgi::ClosePipeFds(){
	for (int& fd : pipe_fds_){
		if (fd != posix::kInvalidFd)
			CloseFd(fd);
	}
}

void	Cgi::CloseInheritedFds(){
	for (int fd = 3; fd < kAssumedMaxFds; fd++)
		::close(fd);
}
