#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "cgi/cgi.hpp"

#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {
	constexpr const char*	kPython = "/usr/bin/python3";
	constexpr const char*	kScriptDir = "./tests/mock_files/cgi/";

	struct ChildResult {
		std::string	output;
		bool		eof = false;
		int			status = -1;
	};

	// Same as OnCgi will do: read until EOF, then reap the child
	ChildResult	ReadUntilEof(int fd, pid_t pid){
		ChildResult	r;
		char		buf[1024];
		pollfd		p{fd, POLLIN, 0};

		while (::poll(&p, 1, 2000) > 0){
			ssize_t n = ::read(fd, buf, sizeof(buf));
			if (n == 0)
				r.eof = true;
			if (n <= 0)
				break;
			r.output.append(buf, n);
		}
		::close(fd);
		::waitpid(pid, &r.status, 0);
		return r;
	}

	CgiInfo	MakeInfo(const std::string& interpreter, const std::string& script, const std::string& query = ""){
		CgiInfo	info;
		info.interpreter = interpreter;
		info.script_name = "/cgi-bin/" + script;
		info.query_string = query;
		info.script_filename = kScriptDir + script;
		return info;
	}

	HttpRequest	MakeRequest(){
		HttpRequest	req{};
		req.method_ = HttpMethod::Get;
		req.version_ = "HTTP/1.1";
		req.headers_ = {{"host", "localhost"}, {"authorization", "secret"}, {"proxy", "evil"}};
		return req;
	}

	bool	HasLine(const std::string& out, const std::string& line){
		return out.find(line + "\n") != std::string::npos;
	}
}

TEST_SUITE("Cgi") {
	ConnInfo		conn{"127.0.0.1", "localhost", "8080"};
	ServerConfig	cfg;

	TEST_CASE("Script runs, pipe reaches EOF, exit status 0") {
		Cgi	cgi(conn, MakeRequest(), MakeInfo(kPython, "env.py"), cfg, "test");
		REQUIRE(cgi.SpawnChild() == HttpStatusCode::OK);

		ChildResult	r = ReadUntilEof(cgi.pipe_fd()[0], cgi.process_num());
		CHECK(r.eof);	// false -> parent kept pipe_fd_[1] open
		CHECK_FALSE(r.output.empty());
		CHECK(WIFEXITED(r.status));
		CHECK(WEXITSTATUS(r.status) == 0);
	}

	TEST_CASE("Read end is non-blocking") {
		Cgi	cgi(conn, MakeRequest(), MakeInfo(kPython, "env.py"), cfg, "test");
		REQUIRE(cgi.SpawnChild() == HttpStatusCode::OK);

		CHECK((::fcntl(cgi.pipe_fd()[0], F_GETFL) & O_NONBLOCK) != 0);
		ReadUntilEof(cgi.pipe_fd()[0], cgi.process_num());
	}

	TEST_CASE("Environment and unsafe headers") {
		Cgi	cgi(conn, MakeRequest(), MakeInfo(kPython, "env.py", "a=1"), cfg, "test");
		REQUIRE(cgi.SpawnChild() == HttpStatusCode::OK);

		std::string out = ReadUntilEof(cgi.pipe_fd()[0], cgi.process_num()).output;
		CHECK(HasLine(out, "GATEWAY_INTERFACE=CGI/1.1"));
		CHECK(HasLine(out, "REQUEST_METHOD=GET"));
		CHECK(HasLine(out, "QUERY_STRING=a=1"));
		CHECK(HasLine(out, "SCRIPT_NAME=/cgi-bin/env.py"));
		CHECK(HasLine(out, "SERVER_PORT=8080"));
		CHECK(HasLine(out, "HTTP_HOST=localhost"));
		CHECK(HasLine(out, "HTTP_AUTHORIZATION=<unset>"));
		CHECK(HasLine(out, "HTTP_PROXY=<unset>"));
	}

	TEST_CASE("Child runs in the script directory") {
		Cgi	cgi(conn, MakeRequest(), MakeInfo(kPython, "env.py"), cfg, "test");
		REQUIRE(cgi.SpawnChild() == HttpStatusCode::OK);

		std::string out = ReadUntilEof(cgi.pipe_fd()[0], cgi.process_num()).output;
		CHECK(out.find("tests/mock_files/cgi\n") != std::string::npos);
	}

	TEST_CASE("Inherited fds are closed in the child") {
		int	extra = ::dup(STDIN_FILENO); // Stands for a client socket
		REQUIRE(extra >= 3);

		Cgi	cgi(conn, MakeRequest(), MakeInfo(kPython, "fd_check.py", std::to_string(extra)), cfg, "test");
		REQUIRE(cgi.SpawnChild() == HttpStatusCode::OK);

		std::string out = ReadUntilEof(cgi.pipe_fd()[0], cgi.process_num()).output;
		CHECK(out == "closed\n");
		::close(extra);
	}

	TEST_CASE("Missing script -> no output, non-zero exit (502)") {
		Cgi	cgi(conn, MakeRequest(), MakeInfo(kPython, "nope.py"), cfg, "test");
		REQUIRE(cgi.SpawnChild() == HttpStatusCode::OK);

		ChildResult	r = ReadUntilEof(cgi.pipe_fd()[0], cgi.process_num());
		CHECK(r.eof);
		CHECK(r.output.empty());
		CHECK(WEXITSTATUS(r.status) != 0);
	}

	TEST_CASE("Missing interpreter -> execve fails, exit 1 (502)") {
		Cgi	cgi(conn, MakeRequest(), MakeInfo("/no/such/python", "env.py"), cfg, "test");
		REQUIRE(cgi.SpawnChild() == HttpStatusCode::OK);

		ChildResult	r = ReadUntilEof(cgi.pipe_fd()[0], cgi.process_num());
		CHECK(r.eof);
		CHECK(r.output.empty());
		CHECK(WEXITSTATUS(r.status) == 127);
	}
}
