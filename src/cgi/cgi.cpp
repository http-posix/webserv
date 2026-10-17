#include "cgi/cgi.hpp"
#include "utils/logger/logger.hpp"
#include "io/event_loop/signal_handler.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <sys/wait.h>
#include <csignal>
#include <cstring>
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
	
	if (::pipe(pipe_fd_.data()) == kInvalidValue){
		LOG_ERROR("Failed pipe() ", srv_id_);
		return HttpStatusCode::InternalServerError;
	}
	pid_ = ::fork();
	if (pid_ == kInvalidValue){
		LOG_ERROR("Failed fork() ", srv_id_);
		ClosePipeFds();
		return HttpStatusCode::InternalServerError;
	}
// who check access to script - here or in http parser?
// TODO: access(F_OK) -> 404, access(R_OK) -> 403. Do it BEFORE pipe() (here or router), otherwise missing script = 502
	if (pid_ == kChildProc){
		if (dup2(pipe_fd_[1], STDOUT_FILENO) == kInvalidValue){
			LOG_ERROR("dup2() failed",  srv_id_);
			std::_Exit(1);
		}
		CloseInheritedFds();
		if (chdir(script_dir_.c_str()) == kInvalidValue)
			std::_Exit(1);
		Signal::SetupDefaultSignals();
		execve(interpreter_.c_str(), argv_, cgi_env_.Envp());
		std::_Exit(127);
	}
	CloseFd(pipe_fd_[1]);
	if (!SetPipeFd(pipe_fd_[0])){
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

const std::array<int, 2>&	Cgi::pipe_fd() const noexcept{
	return pipe_fd_;
}

const pid_t&	Cgi::process_num() const noexcept{
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
	fd = kInvalidValue;
}

void	Cgi::ClosePipeFds(){
	for (int& fd : pipe_fd_){
		if (fd != kInvalidValue)
			CloseFd(fd);
	}
}

void	Cgi::CloseInheritedFds(){
	for (int fd = 3; fd < kMaxFd; fd++)
		::close(fd);
}

bool	Cgi::SetPipeFd(int& fd){
	int ret_code = ::fcntl(fd, F_SETFL, O_NONBLOCK);
	if (ret_code == kInvalidValue){
		LOG_ERROR("fcntl(F_SETFL) failed on pipe fd " + std::to_string(fd) + ": " + std::string(strerror(errno)), srv_id_);
		return false;
	}
	return true;
}
