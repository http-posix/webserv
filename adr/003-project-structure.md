# ADR 003: Project structure — headers alongside sources, unified test rule

**Date:** 2026-05-20
**Deciders:** Tetiana

---

## Context

File layout and test conventions need to be set up before the codebase
grows — otherwise the build flow becomes inconsistent and hard to fix later.

## Decision

`.hpp` files live in the same directory as their `.cpp` counterpart —
dependent parts stay together, easier to navigate and maintain.

`tests/` holds two types of files:
- `*_test.cpp` — verify logic with `assert` or `if` + error message, clear pass/fail signal
- `demo_*.cpp` — show a subsystem working end-to-end, output read manually

`doctest.h` is included in `tests/` — header-only, no build configuration,
used when `assert` is not enough.

`make test` compiles and runs both. Larger integration scenarios
that require manual setup get a dedicated Makefile target.

## Alternatives

- `include/` + `src/` split — rejected: overkill for an application, not a library

## Consequences

\+ Open any module folder — both files are there  
\+ Adding a test requires one `.cpp` file, no framework configuration  
\- `demo_*.cpp` output must be read manually to verify correctness