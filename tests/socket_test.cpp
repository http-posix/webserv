#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "socket/socket.hpp"
#include <cerrno>
#include <fcntl.h>

static bool isFdOpen(int fd) {
	int flags = fcntl(fd, F_GETFL);
	if (flags != -1)
		return true;
	return errno != EBADF;
}

TEST_CASE("Socket: default constructor creates valid fd") {
	Socket s;
	CHECK(s.socket_fd() > 0);
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

TEST_CASE("Socket: wrap constructor takes ownership of fd") {
	int raw = ::socket(AF_INET, SOCK_STREAM, 0);
	REQUIRE(raw >= 0);
	Socket b = Socket::adopt(raw);
	CHECK(b.socket_fd() == raw);
}

TEST_CASE("Socket: copy is deleted (must not compile)") {
	// Uncomment to verify compile-time enforcement:
	// Socket a;
	// Socket b = a;  // Expected: compile error
	CHECK(true);
}

TEST_CASE("Socket: move constructor transfers ownership") {
	Socket a;
	int fd = a.socket_fd();

	Socket b = std::move(a);

	CHECK(a.socket_fd() == -1); // Source is no longer the owner
	CHECK(b.socket_fd() == fd); // New owner holds the fd
}

TEST_CASE("Socket: move constructor does not open a new fd") {
	Socket a;
	int fd = a.socket_fd();
	CHECK(isFdOpen(fd));

	Socket b = std::move(a);
	CHECK(!isFdOpen(a.socket_fd()));
	CHECK(isFdOpen(fd)); // Still one fd, moved to b
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

	Socket& ref = a;
	a = std::move(ref); // Must not crash or double-close

	// Behavior after self-move is unspecified by the standard,
	// But this guard in operator= prevents undefined behavior:
	// if (this != &other)
	CHECK(true);
}

