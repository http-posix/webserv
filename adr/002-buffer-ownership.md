# ADR 002: Buffer ownership — request buffer kept on the parser side

**Date:** 2026-05-20
**Deciders:** Tetiana, Zacharia

---

## Context

Data arrives in chunks via `recv()`. Someone has to accumulate
incomplete bytes between poll cycles.

## Decision

Buffer lives inside `HttpParser` (SRP).
`Connection` passes chunks in, gets parse status back — nothing more.

## Alternatives

- `Connection` owns the buffer and passes accumulated data to parser
  — rejected: `Connection` would need to know when the buffer is
  "complete enough" to hand over, which is HTTP parsing logic

## Consequences

\+ `Connection` has no knowledge of HTTP framing  
\+ Parser state is self-contained  
\- Buffer grows unbounded if a request never completes (slowloris attack) —
  handled by connection timeout in the event loop