# ADR 001: I/O multiplexing — poll() over select(), epoll(), kqueue()

**Date:** 2026-05-20
**Deciders:** Tetiana, Zacharia, Johanna

---

## Context

The event loop must monitor multiple file descriptors (listening socket,
client connections, CGI pipes) simultaneously without blocking.
The project explicitly allows: `select`, `poll`, `epoll_*`, `kqueue`/`kevent`.

The server must compile and run on both Linux (project evaluation machines)
and macOS (local development without Docker).

## Decision

`poll()` is POSIX — the same code compiles and runs on both macOS and Linux
without conditional compilation.

## Consequences

The event loop code is portable and identical on Linux and macOS.

`poll()` does not scale to high connection counts — known and accepted trade-off.