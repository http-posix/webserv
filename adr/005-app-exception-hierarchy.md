# ADR 004: Exception hierarchy — AppException for fatal server errors

**Date:** 2026-05-20
**Deciders:** Tetiana

---

## Context

The server has two failure categories that need different responses:
fatal errors (shut everything down) and connection errors (close that client, keep running).
Without a defined hierarchy, catch sites would need to handle `std::exception` everywhere
or parse error strings — both get messy as the codebase grows.

## Decision

`AppException : public std::runtime_error` is the base for all fatal errors.
Subclasses (`SocketException`, `ConfigException`) act as self-documentation
and type-system tags for now — no added behaviour yet, but specific catch blocks
become possible without parsing strings.

`std::bad_alloc` is caught separately in `main()` with `write()` to stderr —
heap allocation in an out-of-memory handler is not safe.

## Alternatives

- Catch `std::exception` everywhere — too broad, hides intent

## Consequences

\+ One catch in `main()` handles all fatal errors  
\+ Adding a new category = new subclass, existing catch sites unchanged  
\- Subclasses currently share the same behaviour — hierarchy exists for clarity, not functionality