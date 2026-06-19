# ADR 005: Logger design — Meyers Singleton with compile-time flags

**Date:** 2026-05-20
**Deciders:** Tetiana

---

## Context

The server needs a logger accessible from any part of the codebase.
It optionally writes to a file, so a single shared instance
must stay alive for the whole run.

## Decision

Singleton over free functions or a global variable.
The logger holds state: an open file stream and an active/inactive flag.
A Meyers Singleton (`static Logger instance` inside `GetInstance()`)
makes ownership explicit, requires no manual `delete`, and guarantees
correct destruction at the end of the program.


`#define` macros over direct method calls.
`__FILE__`, `__LINE__`, and `__func__` expand at the call site.
Wrapping them in a regular function would make every log line
point to `logger.cpp` instead of where the log was actually written.

```cpp
LOG_INFO("server started");
// expands to: Logger::GetInstance().Log(Level::INFO, __FILE__, __LINE__, __func__, "server started")
```

Compile-time flags over runtime switching.
The server does not change log behaviour while running.
A runtime flag would add a branch to every log call for no benefit.

```
make              → logging off
make DBG=1        → all levels to stderr
make DBG=1 LOG=1  → all levels to stderr and to log/ directory
```

Default build has all logging off.
The flag strategy will be reviewed once the logger is in active use.

Static methods for stateless operations (`PrintMsg`).
Methods that do not read or modify logger state are marked `static`
to make this explicit and to allow calling them without `GetInstance()`.

## Consequences

Changing log behaviour requires recompilation. Acceptable at this scale.

Any translation unit that needs logging includes `logger.hpp` and uses
`LOG_*` macros — no setup, no passing instances around.

The Singleton pattern is a natural fit for a logger — one instance,
shared state, controlled lifetime. Taken also as a chance to practice
the pattern in a real codebase.