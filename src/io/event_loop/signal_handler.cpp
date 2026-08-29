#include "io/event_loop/signal_handler.hpp"

namespace Signal{

	void HandleShutdown(int /*signal*/){
		g_signal_running = 0;
	}

	// Call once in main(), before run_app()
	void SetupSignalHandlers(){
		std::signal(SIGINT, HandleShutdown);
		std::signal(SIGTERM, HandleShutdown);
		std::signal(SIGPIPE, SIG_IGN);
	}

	// Call in the CGI child, after fork() and before execve() —
	// see minishell(Tetiana) for the same pattern.
	void SetupDefaultSignals(){
		std::signal(SIGINT, SIG_DFL);
		std::signal(SIGTERM, SIG_DFL);
		std::signal(SIGPIPE, SIG_DFL);
	}
}
