#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "io/event_loop/signal_handler.hpp"

#include <csignal>
#include <unistd.h>

TEST_SUITE("SignalHandling") {

    TEST_CASE("Handle SIGINT shutdown") {
        Signal::g_signal_running = 1;
        Signal::SetupSignalHandlers();

        std::raise(SIGINT);

        CHECK(Signal::g_signal_running == 0);
    }

    TEST_CASE("Handle SIGTERM shutdown") {
        Signal::g_signal_running = 1;
        Signal::SetupSignalHandlers();

        std::raise(SIGTERM);

        CHECK(Signal::g_signal_running == 0);
    }

    TEST_CASE("SIGPIPE is ignored") {
        Signal::SetupSignalHandlers();
        int pipe_fds[2];
        if (pipe(pipe_fds) == 0) {
            
			close(pipe_fds[0]); // close read-end
			
			// if SIGPIPE doesn't ignored, next line kill the process
            ssize_t bytes = write(pipe_fds[1], "test", 4);

            CHECK(bytes == -1);
            CHECK(errno == EPIPE);
            close(pipe_fds[1]);
        }
    }

    TEST_CASE("Signals reset to default") {
        Signal::SetupSignalHandlers();
        Signal::SetupDefaultSignals();
        
        struct sigaction sa;
        sigaction(SIGINT, nullptr, &sa);

        CHECK(sa.sa_handler == SIG_DFL);
    }
}
