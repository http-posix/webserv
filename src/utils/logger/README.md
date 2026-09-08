## Logger

A simple logger for development and debugging.  
Implemented as a Meyers Singleton — a function-local static instance, constructed on first use and destroyed at exit.

**Build flags:**  
`make` — INFO / WARN / ERROR to terminal. LOG_DEBUG compiles to nothing.
`make DBG=1` — logs to terminal  
`make DBG=1 LOG=1` — logs to terminal and to log/ directory  

**PrintMsg:**  
Logger::PrintMsg("message") — static helper for standard output, no instance required.

**Macros:**  
LOG_DEBUG("message") — development only
LOG_INFO("message") — clean status line
LOG_WARN("message") — the client or the environment misbehaved, we handled it
LOG_ERROR("message") — webserv itself failed

DEBUG / INFO / ERROR macro automatically records at the call site:
 - file name
 - line number

