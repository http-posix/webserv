# ADR 006: Socket fd ownership — RAII wrapper with `adopt` factory

**Date:** 2026-06-01
**Deciders:** Tetiana

---

## Context

Raw `int` fd has no ownership semantics — nothing prevents double-close
or a leak on an error path. A double-close is hard to trace: the OS reuses
fds, so the second `close()` silently kills a different client's connection.
In a test with a public `Socket(int fd)` constructor, wrapping the same fd
twice reproduced this: `strace` showed `EBADF` on the second `close()`.
Valgrind missed it because it only reports fds still open at exit,
not erroneous closes mid-run.

## Decision

`Socket` is a non-copyable RAII wrapper. Fd is closed in the destructor,
exactly once. The only way to construct a `Socket` from a raw fd is through
the `Socket::adopt(fd)` factory — a public constructor was tried first
and allowed accidental double-wrapping (`Socket b{a.socket_fd()}`), which
reproduced the original bug.

## Alternatives

- Raw `int` everywhere — no ownership, easy to misuse, already caused bugs
- Public constructor — tried, rejected: nothing stops double-wrapping at compile time

## Consequences

\+ Double-close is impossible to write by accident  
\+ Leak-free on error paths without manual `close()`  
\- Every syscall that returns a raw fd (`socket()`, `accept()`) needs
  an explicit `Socket::adopt()` call — one extra line, acceptable