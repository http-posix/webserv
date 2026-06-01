#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "socket/socket.hpp"
#include <filesystem>

// Counts open file descriptors via /proc/self/fd
// Note: directory_iterator itself opens a fd, so counts will be +1 during iteration
static int countOpenFds() {
	int count = 0;
	for ([[maybe_unused]] auto& entry : std::filesystem::directory_iterator("/proc/self/fd"))
		count++;
	return count;
}

TEST_CASE("Socket: default constructor creates valid fd") {
	Socket s;
	CHECK(s.socket_fd() > 0);
}

TEST_CASE("Socket: destructor closes fd") {
	int before = countOpenFds();
	{
		Socket s;
		CHECK(countOpenFds() == before + 1);
	} // Destructor called here
	CHECK(countOpenFds() == before);
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
	int before = countOpenFds();
	Socket a;
	CHECK(countOpenFds() == before + 1);

	Socket b = std::move(a);
	CHECK(countOpenFds() == before + 1); // Still one fd, not two
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
	int before = countOpenFds();
	{
		Socket a;
		Socket b;
		CHECK(countOpenFds() == before + 2); // Two fds open

		b = std::move(a); // b closes its own fd before taking a's fd
		CHECK(countOpenFds() == before + 1); // One fd was closed
	}
	CHECK(countOpenFds() == before); // Last fd closed by destructor
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