## Logger

A simple logger for development and debugging.  
Implemented as a Meyers Singleton — a function-local static instance, constructed on first use and destroyed at exit.

**Build flags:**  
`make` — INFO / WARN / ERROR to terminal. LOG_DEBUG compiles to nothing.  
`make DBG=1` — adds DEBUG level
`make DBG=1 LOG=1` — adds a copy in log/ directory  

**PrintMsg:**  
Logger::PrintMsg("message") — static helper for standard output.

**Macros:**  
LOG_DEBUG("message") — development only
LOG_INFO("message") — normal lifecycle events
LOG_WARN("message") — the client or the environment misbehaved, we handled it
LOG_ERROR("message") — webserv itself failed

**Usage**
```
#include "utils/logger/logger.hpp"

LOG_INFO("server started");							// without context
LOG_DEBUG("request fd=6 recv_bytes=77", srv_id_);	// with server context

```

Every macro takes the message first and an optional context string second.
The second argument identifies which server the record belongs to. It is built once per Server and stored in srv_id_:
```
[127.0.0.1:8080 fd=4]
```

Classes that own a srv_id_ (Server, Connection) pass it on every call. Classes that cannot know it (PollManager, Socket, poll() error handling) log without context — they only report the bare fact, and the caller one level up adds the circumstances.

The context is rendered dim so it stays visually behind the message. When it is omitted, nothing is printed in its place.

Additionally each macro automatically records at the call site:
 - file name
 - line number

**Output format**
```
20:40:36.084 [--DEBUG--] connection.cpp:71:  [127.0.0.1:8080 fd=3] state - Complete fd=5
```
Run `make demo` to see it in action.

**Notes**
 - STDERR contains ANSI codes, while file output does not.
 - PrintMsg writes to stdout and is meant for user-facing greetings, not for log records.
