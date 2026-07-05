# ADR 007: Non-blocking sockets, set by construction

**Date:** 2026-07-05
**Deciders:** Tetiana

---

## Context

Every socket used by the event loop — the listening socket and every client socket accepted later — must be non-blocking. The whole loop is built around a single `poll()` call, and no syscall on any of these sockets is allowed to block it, even for a short time. There is no case in this design where a blocking `Socket` is acceptable.

`Socket` has two constructors by design: the default constructor (used once, for the listening socket) and a private constructor used by `adopt()` (used on every `accept()`, for client sockets). These two call sites have very different failure costs — the listening socket failing means the server cannot run at all; a client socket failing on one connection is routine and must not affect anything else.

## Decision

Non-blocking mode is set inside both of `Socket`'s constructors, right after the fd is obtained, before the constructor returns — not by a separate public method that callers must remember to invoke.

The two constructors handle failure differently, matching their call sites:
- default constructor (listening socket): throws `ServerException` on any failure, including a failed non-blocking setup, and closes the fd manually first.
- private constructor via `adopt()` (client sockets): never throws. `adopt()` requires the caller to pass an already-valid fd (`fd >= 0`, checked with `assert` — a programmer contract, not a runtime check) and returns a `Socket` that may still end up invalid (`socket_fd() == kInvalidFd`) if the internal fcntl setup fails. The caller (event loop) must check this before registering the fd with `poll()`.

## Alternatives

- Public method (e.g. `SetNonBlockingMode()`) called explicitly by every place that creates a `Socket` — rejected: relies on every call site remembering to call it, easy to forget, nothing catches it at compile time.
- Static factory functions for non-blocking setup — considered, rejected: adopt() already serves this purpose, and factories wouldn't simplify the different error-handling paths.
- Throwing from `adopt()` too, for consistency — rejected: would force the event loop to pay exception-unwinding cost on a hot path (every accepted connection) for a condition that is rare but routine in that context.

## Consequences

+ Every `Socket` in the project is non-blocking from the moment it exists — impossible to forget at a call site.
+ No separate step needed in `Server`'s constructor or the future `Connection` code.
+ No exception cost on the accept() hot path; listening socket still fails fast and loud.
- Constructor does slightly more work and requires manual fd cleanup on error.
- The event loop must remember to check the returned Socket's validity after `adopt()` — not enforced at compile time, only by contract.