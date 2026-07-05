#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "socket/socket.hpp"
#include <cerrno>
#include <fcntl.h>
#include <type_traits>

static bool isFdOpen(int fd) {
	int flags = fcntl(fd, F_GETFL);
	if (flags != -1)
		return true;
	return errno != EBADF;
}

TEST_CASE("Socket: default constructor creates valid fd") {
	Socket s;
	CHECK(s.socket_fd() >= 0);
	CHECK(isFdOpen(s.socket_fd()));
}

TEST_CASE("Socket: destructor closes fd") {
	int fd;
	{
		Socket s;
		fd = s.socket_fd();
		CHECK(isFdOpen(fd));
	} // Destructor called here
	CHECK(!isFdOpen(fd));
}

TEST_CASE("Socket: adopt() takes ownership of fd") {
	int raw = ::socket(AF_INET, SOCK_STREAM, 0);
	REQUIRE(raw >= 0);
	Socket b = Socket::adopt(raw);
	CHECK(b.socket_fd() == raw);
}

TEST_CASE("Socket: copy is deleted (must not compile)") {
	static_assert(!std::is_copy_constructible_v<Socket>, "Socket must not be copyable");
	static_assert(!std::is_copy_assignable_v<Socket>, "Socket must not be copy-assignable");
}

TEST_CASE("Socket: move constructor transfers fd ownership") {
	Socket a;
	int fd = a.socket_fd();
	CHECK(isFdOpen(fd));

	Socket b = std::move(a);

	CHECK(a.socket_fd() == -1);
	CHECK(b.socket_fd() == fd);
	CHECK(isFdOpen(fd));
}

TEST_CASE("Socket: move assignment transfers ownership") {
	Socket a;
	Socket b;
	int fd_a = a.socket_fd();

	b = std::move(a);

	CHECK(a.socket_fd() == -1);
	CHECK(b.socket_fd() == fd_a);
}

TEST_CASE("Socket: move assignment closes current fd") {
	int fd_b;
	{
		Socket a;
		Socket b;
		fd_b = b.socket_fd();
		CHECK(isFdOpen(fd_b));

		b = std::move(a); // b closes its own fd before taking a's fd
		CHECK(!isFdOpen(fd_b));
	}
}

TEST_CASE("Socket: self move assignment is safe") {
	Socket a;
	int fd = a.socket_fd();

	Socket& ref = a;
	a = std::move(ref);

	// if guard (this != &other) в operator= works correctly,
	// self-move cann't broke
	CHECK(a.socket_fd() == fd);
	CHECK(isFdOpen(fd));
}

TEST_CASE("Socket: default constructor produces non-blocking socket") {
	Socket s;
	REQUIRE(s.socket_fd() >= 0);

	int flags = fcntl(s.socket_fd(), F_GETFL, 0);
	REQUIRE(flags != -1);
	CHECK((flags & O_NONBLOCK) != 0);
}

TEST_CASE("Socket: adopted socket is non-blocking") {
	int raw_fd = ::socket(AF_INET, SOCK_STREAM, 0);
	REQUIRE(raw_fd >= 0);

	Socket s = Socket::adopt(raw_fd);
	REQUIRE(s.socket_fd() >= 0);

	int flags = fcntl(s.socket_fd(), F_GETFL, 0);
	REQUIRE(flags != -1);
	CHECK((flags & O_NONBLOCK) != 0);
}

// Produce LOG_ERROR()
TEST_CASE("Socket: adopt() with invalid fd does not throw, ends invalid") {
	int fd = ::socket(AF_INET, SOCK_STREAM, 0);
	REQUIRE(fd >= 0);
	::close(fd);

	Socket s = Socket::adopt(fd);
	CHECK(s.socket_fd() == -1);
}
