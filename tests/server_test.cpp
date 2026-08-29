#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "app/server/server.hpp"
#include "../tests/mock_files/mock_config.hpp"
#include "utils/app_exception/app_exception.hpp"
// #include "config/mock_config.hpp"

#include <type_traits>

TEST_SUITE("Server") {

	TEST_CASE("constructs successfully on loopback with ephemeral port") {
		// port 0 -> OS will choose free port
		CHECK_NOTHROW(Server srv("127.0.0.1", 0));
	}

	TEST_CASE("fd() returns a valid (non-negative) descriptor") {
		Server srv("127.0.0.1", 0);
		CHECK(srv.fd() >= 0);
	}

	TEST_CASE("two servers can bind to two different ephemeral ports") {
		Server srv1("127.0.0.1", 0);
		Server srv2("127.0.0.1", 0);
		CHECK(srv1.fd() != srv2.fd());
	}

	// it is two active server, not TIME_WAIT
	// expect to fail with EADDRINUSE + LOG_ERROR("bind()")
	TEST_CASE("binding the same fixed port twice throws (EADDRINUSE)") {
		const uint16_t test_port = 54321;

		Server first("127.0.0.1", test_port);
		CHECK_THROWS_AS(Server second("127.0.0.1", test_port), ServerException);
	}

	// produce LOG_ERROR("getaddrinfo()")
	TEST_CASE("invalid/unresolvable host throws via getaddrinfo failure") {
		CHECK_THROWS_AS(
			Server srv("this.host.does.not.resolve.invalid", 8080),
			ServerException
		);
	}

	// Template Metaprogramming
	// Check with <type_traits> if move semantics are supported
	TEST_CASE("Server is move-constructible and move-assignable") {
		static_assert(std::is_move_constructible<Server>::value,
			"Server must be move-constructible");
		static_assert(std::is_move_assignable<Server>::value,
			"Server must be move-assignable");
	}

	// Check with <type_traits> if copy semantics are disabled
	TEST_CASE("Server is NOT copyable (compile-time contract)") {
		static_assert(!std::is_copy_constructible<Server>::value,
			"Server must not be copy-constructible");
		static_assert(!std::is_copy_assignable<Server>::value,
			"Server must not be copy-assignable");
	}

	TEST_CASE("moved-from Server transfers fd ownership without double-close") {
		Server srv1("127.0.0.1", 0);
		int original_fd = srv1.fd();

		Server srv2(std::move(srv1));

		CHECK(srv1.fd() == -1);
		CHECK(srv2.fd() == original_fd);
	}

	TEST_CASE("server_host() returns the host passed to constructor") {
		Server srv("127.0.0.1", 0);
		CHECK(srv.server_host() == "127.0.0.1");
	}

	TEST_CASE("server_port() returns the port passed to constructor") {
		const uint16_t test_port = 54322;
		Server srv("127.0.0.1", test_port);
		CHECK(srv.server_port() == test_port);
	}
}

TEST_SUITE("CreateListeners") {

	TEST_CASE("creates one listener per unique host:port pair") {
		Config cfg = MockConfig::SingleServer("127.0.0.1", {8081, 8082});
		std::vector<Server> listeners = CreateListeners(cfg);
		CHECK(listeners.size() == 2);
	}

	// Produce LOG_ERROR("Duplicate configuration detected")
	TEST_CASE("throws on duplicate host:port across servers") {
		Config cfg = MockConfig::TwoServersSameSettings("127.0.0.1", 8083);
		CHECK_THROWS_AS(CreateListeners(cfg), ServerException);
	}

	// doctest.h hasn't ASSERT_DEATH(), for this we need gtest.
	// TEST_CASE("empty config produces empty listener list") {
	// 	Config cfg = MockConfig::Empty();
	// 	std::vector<Server> listeners = CreateListeners(cfg);
	// 	CHECK(listeners.empty());
	// }
}
