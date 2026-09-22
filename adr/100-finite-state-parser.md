# ADR 100: Feed-based HttpParser with Finite State Machine.

**Date:** 2026-06-08
**Recorded:** 2026-09-22  <!-- only if written later -->
**Deciders:** Zach, Tetiana<!-- who made this call -->

---

## Context
A socket read may deliver a request in any number of pieces: `"GET / "`,
then `"HTTP/1.1\r\nHost: x"`, then the rest. The HTTP parser therefore cannot
be a function that consumes one complete message. It must accept whatever
arrives, process/validate as much as it can and signal whether the request
is either (in)valid and/or (in)complete.

## Decision
The HttpParser has an internal and an external state. The internal state 
simulates a Finite State Machine and the external state communicates
with the event loop whether the process of parsing is (in)complete or
(in)valid. The `feed()` function allows for appending new content to the parser.
Partial content stays buffered inside the parser as was decided in ADR002.

## Alternatives
A one-shot parser that handles the entire request only once it is complete.
This is inefficient (since you have to check whether the request is complete
everytime new pieces of information are added) and also does not allow for
checking of possible invalid request while incomplete.
<!-- - option — why not (skip if none worth mentioning) -->

## Consequences
+ Allows for validation of request line and headers without the request being complete.
- Parser keeps the entire request into memory.
- Appending string in C++ makes for copying which can be slow in extreme cases.
<!-- \+ easier -->
<!-- \- trade-off accepted -->
