# ADR 004: Test conventions

**Date:** 2026-05-20
**Deciders:** Tetiana

---

## Context

The project needs a consistent way to test logic with a clear pass/fail
signal, while keeping setup friction low — no build system to wrangle
just to add one test.

## Decision

`tests/` holds two types of files:
- `*_test.cpp` — verify logic with `assert` or `if` + error message, clear pass/fail signal
- `demo_*.cpp` — show a subsystem working end-to-end, output read manually

`doctest.h` is included in `tests/` — header-only, no build configuration,
used when plain `assert` is not enough.

`make test` compiles and runs both. Larger integration scenarios that
require manual setup get a dedicated Makefile target.

## Alternatives

- Catch2 / GoogleTest — rejected: require build/link integration, heavier than the project needs
- Plain `assert` only — insufficient when richer checks/messages are needed; doctest covers that gap while staying header-only

## Consequences

\+ Adding a test requires one `.cpp` file, no framework configuration  
\+ Header-only keeps the Makefile simple  
\- `demo_*.cpp` output must be read manually to verify correctness