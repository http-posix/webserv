#include "cgi/cgi.hpp"
#include "http/http_status_code.hpp"
#include "utils/logger/logger.hpp"
#include "io/event_loop/signal_handler.hpp"
#include "utils/posix/posix.hpp"

#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>

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

	bool	RedirectStdinToDevNull(){
		int	dev_null = ::open("/dev/null", O_RDONLY);
		if (dev_null == -1 || ::dup2(dev_null, STDIN_FILENO) == -1)
			return false;
		::close(dev_null);
		return true;
	}
}

/* ========================================================================== */
/*                          Constructors & Destructors                        */
/* ========================================================================== */

Cgi::Cgi(const ConnInfo& info,
		const HttpRequest& req,
		const CgiRequest& cgi_info,
		const std::string& srv_id):
	srv_id_(srv_id),
	cgi_env_(info, req, cgi_info),
	interpreter_(cgi_info.interpreter),
	has_body_(!req.body_.empty())
{
	BuildArgv(cgi_info.script_filename);
}

/* ========================================================================== */
/*                               Public Methods                               */
/* ========================================================================== */

HttpStatusCode	Cgi::SpawnChild(){
	if (!OpenPipe(stdout_pipe_, kReadEnd, "stdout_pipe"))
		return HttpStatusCode::InternalServerError;
	if (has_body_ && !OpenPipe(stdin_pipe_, kWriteEnd, "stdin_pipe")){
		ClosePipeFds(stdout_pipe_);
		return HttpStatusCode::InternalServerError;
	}

	pid_ = ::fork();
	if (pid_ == posix::kInvalidPid){
		LOG_ERROR("Failed fork() ", srv_id_);
		ClosePipeFds(stdout_pipe_);
		ClosePipeFds(stdin_pipe_);
		return HttpStatusCode::InternalServerError;
	}
	// who check access to script - here or in http parser?
	// TODO: access(F_OK) -> 404, access(R_OK) -> 403. 
	// Do it BEFORE pipe() (here or router), otherwise missing script = 502
	if (pid_ == posix::kForkChild){
		if (::dup2(stdout_pipe_[kWriteEnd], STDOUT_FILENO) == posix::kSyscallError){
			LOG_ERROR("dup2() failed",  srv_id_);
			std::_Exit(1);
		}
		if (has_body_){
			if (::dup2(stdin_pipe_[kReadEnd], STDIN_FILENO) == posix::kSyscallError){
				LOG_ERROR("dup2() failed", srv_id_);
				std::_Exit(1);
			}
		}
		CloseInheritedFds();
		if (chdir(script_dir_.c_str()) == posix::kSyscallError)
			std::_Exit(1);
		Signal::SetupDefaultSignals();
		if (!has_body_ && !RedirectStdinToDevNull())
			std::_Exit(1);

		execve(interpreter_.c_str(), argv_, cgi_env_.Envp());
		std::_Exit(127);
	}
	CloseFd(stdout_pipe_[kWriteEnd]);
	if (has_body_)
		CloseFd(stdin_pipe_[kReadEnd]);
	return HttpStatusCode::OK;
}

/* ========================================================================== */
/*                             Accessors & Mutators                           */
/* ========================================================================== */

int	Cgi::pipe_read_fd() const noexcept{
	return stdout_pipe_[kReadEnd];
}

pid_t	Cgi::child_pid() const noexcept{
	return pid_;
}

int	Cgi::pipe_write_fd() const noexcept{
	return stdin_pipe_[kWriteEnd];
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

void	Cgi::ClosePipeFds(std::array<int, 2>& fds){
	for (int& fd : fds){
		if (fd != posix::kInvalidFd)
			CloseFd(fd);
	}
}

void	Cgi::CloseInheritedFds(){
	for (int fd = 3; fd < kAssumedMaxFds; fd++)
		::close(fd);
}

bool	Cgi::OpenPipe(std::array<int, 2>& fds, int server_end, const char* name){
	if (::pipe(fds.data()) == posix::kSyscallError){
		LOG_ERROR(std::string("Failed pipe() for ") + name, srv_id_);
		return false;
	}
	if (!posix::SetNonBlockingMode(fds[server_end], "pipe fd", srv_id_)){
		ClosePipeFds(fds);
		return false;
	}
	return true;
}
