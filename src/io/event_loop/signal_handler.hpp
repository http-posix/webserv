#pragma once

#include <csignal>

/**
 * @namespace Signal
 * @brief Process-wide signal handling for graceful shutdown and CGI child processes.
 *
 */

namespace Signal {
	inline volatile std::sig_atomic_t	g_signal_running = 1;

	void HandleShutdown(int /*signal*/);
	void SetupSignalHandlers();
	void SetupDefaultSignals();
}
