# ADR 101: Commincation between HTTP Parser and Event loop.

**Date:** 2026-06-XX
**Recorded:** 2026-09-22
**Deciders:** Zach, Tetiana

---

## Context
Requests can come in chunks, meaning that the HTTP-parser needs to communicate whether requests are
complete, invalid or incomplete in order to understand what the next move needs to be.

## Decision
The HTTP `feed()` function returns a simple `enum` communicating whether the request is
`invalid`, `incomplete` (`needs_more_data`) or `complete. An additional `error` state is
defined for undefined errors.

## Alternatives

## Consequences
+ Clear and strict communication between Parser and Event Loop.
- `Error` is an ambiguous (and unused) state mainly for debugging.

