#include <sys/poll.h>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "event_loop/poll_manager.hpp"
#include <unistd.h>
#include <poll.h>
#include "logger/logger.hpp"

// ------------------------------------------------------------------
// Watch / Unwatch / SetEvents — no real fds needed here, PollManager
// never validates or opens the fd itself, it just stores the int.
// ------------------------------------------------------------------


// ------------------------------------------------------------------
// LOG_WARN is triggered inside Unwatch()/SetEvents() as part 
// of expected behavior
// ------------------------------------------------------------------

TEST_CASE("Watch adds fd with given events and revents=0"){
	PollManager pm;
	pm.Watch(42, POLLIN);

	const std::vector<pollfd>& entries = pm.poll_fds();
	REQUIRE(entries.size() == 1);
	CHECK(entries[0].fd == 42);
	CHECK(entries[0].events == POLLIN);
	CHECK(entries[0].revents == 0);
}

TEST_CASE("Watch multiple fds appends in insertion order"){
	PollManager pm;
	pm.Watch(100, POLLIN);
	pm.Watch(200, POLLOUT);
	pm.Watch(300, POLLIN);

	const std::vector<pollfd>& entries = pm.poll_fds();
	REQUIRE(entries.size() == 3);
	CHECK(entries[0].fd == 100);
	CHECK(entries[0].events == POLLIN);

	CHECK(entries[1].fd == 200);
	CHECK(entries[1].events == POLLOUT);

	CHECK(entries[2].fd == 300);
	CHECK(entries[2].events == POLLIN);
}

TEST_CASE("Watch() twice on the same fd creates two separate entries — caller's responsibility to avoid this"){
	PollManager pm;
	pm.Watch(100, POLLIN);
	pm.Watch(100, POLLOUT);

	const std::vector<pollfd>& entries = pm.poll_fds();
	CHECK(entries.size() == 2); // documents current behavior: no dedup, no merge
	CHECK(entries[0].events == POLLIN);
	CHECK(entries[1].events == POLLOUT);
}

TEST_CASE("Unwatch removes the target fd"){
	PollManager pm;
	pm.Watch(100, POLLIN);
	pm.Watch(200, POLLIN);
	pm.Watch(300, POLLIN);

	pm.Unwatch(200);

	const std::vector<pollfd>& entries = pm.poll_fds();
	REQUIRE(entries.size() == 2);
	for (const auto& e : entries)
		CHECK(e.fd != 200);
}

TEST_CASE("Unwatch uses swap-and-pop: last element takes the removed slot"){
	// This test documents the swap-and-pop CONTRACT, not just the outcome.
	// If Unwatch's internal strategy ever changes (e.g. back to erase()),
	// this test will correctly fail and flag the behavior change.
	PollManager pm;
	pm.Watch(100, POLLIN);
	pm.Watch(200, POLLIN);
	pm.Watch(300, POLLIN);

	pm.Unwatch(100); // removes index 0 -> back() (fd=300) should move there

	const std::vector<pollfd>& entries = pm.poll_fds();
	REQUIRE(entries.size() == 2);
	CHECK(entries[0].fd == 300);
	CHECK(entries[1].fd == 200);
}

TEST_CASE("Unwatch on a missing fd is a safe no-op"){
	PollManager pm;
	pm.Watch(100, POLLIN);

	pm.Unwatch(999999); // not present — must not throw, must not touch existing entries

	const std::vector<pollfd>& entries = pm.poll_fds();
	REQUIRE(entries.size() == 1);
	CHECK(entries[0].fd == 100);
}

TEST_CASE("SetEvents updates the events mask for an existing fd"){
	PollManager pm;
	pm.Watch(100, POLLIN);
	const std::vector<pollfd>& entries_before = pm.poll_fds();

	REQUIRE(entries_before.size() == 1);
	CHECK(entries_before[0].events == POLLIN);

	pm.SetEvents(100, POLLOUT);

	const std::vector<pollfd>& entries_after = pm.poll_fds();
	REQUIRE(entries_after.size() == 1);
	CHECK(entries_after[0].events == POLLOUT);
}

TEST_CASE("SetEvents on a missing fd is a safe no-op"){
	PollManager pm;
	pm.Watch(100, POLLIN);

	pm.SetEvents(999999, POLLOUT);

	const std::vector<pollfd>& entries = pm.poll_fds();
	REQUIRE(entries.size() == 1);
	CHECK(entries[0].fd == 100);
	CHECK(entries[0].events == POLLIN); // unchanged
}

// ------------------------------------------------------------------
// Poll() — needs real fds to be meaningful. pipe() is CI-safe:
// no network, no ports, fully deterministic, no root required.
// Every timeout below is a small bounded number — never -1.
// A Poll(-1) call with a wrongly-assumed-ready fd would hang the
// CI job until the runner's global timeout kills it.
// ------------------------------------------------------------------

TEST_CASE("Poll() reports a pipe fd as readable after a write"){
	int fds[2];
	REQUIRE(pipe(fds) == 0);
	int read_fd = fds[0];
	int write_fd = fds[1];

	PollManager pm;
	pm.Watch(read_fd, POLLIN);

	const char msg = 'x';
	REQUIRE(write(write_fd, &msg, 1) == 1);

	int ready = pm.Poll(100); // 100ms bound, CI-safe
	REQUIRE(ready == 1);

	const std::vector<pollfd>& entries = pm.poll_fds();
	CHECK((entries[0].revents & POLLIN) != 0);

	close(read_fd);
	close(write_fd);
}

TEST_CASE("Poll() returns 0 (timeout) when nothing is ready"){
	int fds[2];
	REQUIRE(pipe(fds) == 0);
	int read_fd = fds[0];
	int write_fd = fds[1];

	PollManager pm;
	pm.Watch(read_fd, POLLIN);
	// nothing written — read_fd stays not-readable

	int ready = pm.Poll(50); // short bounded timeout
	CHECK(ready == 0);

	close(read_fd);
	close(write_fd);
}

TEST_CASE("Poll() called twice: revents from a stale event does not linger on the next call"){
	// This documents an assumption about ::poll() itself, not something
	// PollManager does: POSIX guarantees revents is overwritten on every
	// call, never accumulated from a previous one — nothing to reset by hand.
	int fds[2];
	REQUIRE(pipe(fds) == 0);
	int read_fd = fds[0];
	int write_fd = fds[1];

	PollManager pm;
	pm.Watch(read_fd, POLLIN);

	const char msg = 'x';
	REQUIRE(write(write_fd, &msg, 1) == 1);

	int first_ready = pm.Poll(100);
	REQUIRE(first_ready == 1);
	CHECK((pm.poll_fds()[0].revents & POLLIN) != 0);

	char buf;
	REQUIRE(read(read_fd, &buf, 1) == 1); // drain the pipe

	int second_ready = pm.Poll(50); // nothing new written this time
	CHECK(second_ready == 0);
	CHECK(pm.poll_fds()[0].revents == 0); // stale POLLIN from the first call is gone

	close(read_fd);
	close(write_fd);
}

TEST_CASE("Poll() reports multiple fds ready at once"){
	int fds_a[2];
	int fds_b[2];
	REQUIRE(pipe(fds_a) == 0);
	REQUIRE(pipe(fds_b) == 0);
	int read_a = fds_a[0];
	int write_a = fds_a[1];
	int read_b = fds_b[0];
	int write_b = fds_b[1];

	PollManager pm;
	pm.Watch(read_a, POLLIN);
	pm.Watch(read_b, POLLIN);

	const char msg = 'x';
	REQUIRE(write(write_a, &msg, 1) == 1);
	REQUIRE(write(write_b, &msg, 1) == 1);

	int ready = pm.Poll(100);
	CHECK(ready == 2);

	const std::vector<pollfd>& entries = pm.poll_fds();
	CHECK((entries[0].revents & POLLIN) != 0);
	CHECK((entries[1].revents & POLLIN) != 0);

	close(read_a);
	close(write_a);
	close(read_b);
	close(write_b);
}

TEST_CASE("Poll() reports a closed peer as POLLHUP and/or POLLIN"){
	// Even on a single OS (Ubuntu), the exact revents bit on peer-close
	// depends on whether unread data remains in the pipe buffer at
	// close() time — a kernel implementation detail, not a POSIX
	// guarantee we should hard-code. Accept either bit.
	int fds[2];
	REQUIRE(pipe(fds) == 0);
	int read_fd = fds[0];
	int write_fd = fds[1];

	PollManager pm;
	pm.Watch(read_fd, POLLIN);

	close(write_fd); // peer closes its end

	int ready = pm.Poll(100);
	REQUIRE(ready == 1);

	const std::vector<pollfd>& entries = pm.poll_fds();
	CHECK(((entries[0].revents & POLLHUP) != 0 || (entries[0].revents & POLLIN) != 0));

	close(read_fd);
}

// ------------------------------------------------------------------
// MANUAL-ONLY tests below are commented out on purpose — they are
// NOT run in GitHub Actions, because their outcome depends on
// timing/ports/peer-behavior the CI container doesn't control.
// This TEST_CASE fires on every run (CI included) so nobody forgets
// these exist and need to be checked by hand on a dev machine.
// ------------------------------------------------------------------

TEST_CASE("Reminder: manual-only PollManager tests exist and need a human"){
	LOG_WARN("\nNB! SOME TESTS CAN'T RUN IN GIT ACTIONS AND NEED TO BE CHECKED MANUALLY");
	CHECK(true); // no real assertion — this test case exists only to emit the reminder
}

/*
TEST_CASE("MANUAL: Poll() detects a real TCP listening socket becoming readable" * doctest::skip(true)){
	// Why manual: binding a real port on a shared CI runner risks
	// "address already in use" flakiness we don't control here.
	// Run manually: start this test, then `nc 127.0.0.1 9090` from
	// another terminal while it's running.
	// TODO: add a script that runs `nc` with a short delay in the
	// background so this doesn't need manual two-terminal timing.
	Server server("127.0.0.1", 9090);
	PollManager pm;
	pm.Watch(server.fd(), POLLIN);

	int ready = pm.Poll(1000);
	REQUIRE(ready == 1);
	const std::vector<pollfd>& entries = pm.poll_fds();
	CHECK((entries[0].revents & POLLIN) != 0);
}*/