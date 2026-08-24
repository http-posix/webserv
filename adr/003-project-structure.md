# ADR 003: Project structure — headers alongside sources

**Date:** 2026-05-20
**Deciders:** Tetiana

---

## Context

File layout needs to be set before the codebase grows — otherwise the
build flow becomes inconsistent and hard to fix later.

## Decision

`.hpp` files live in the same directory as their `.cpp` counterpart —
dependent parts stay together, easier to navigate and maintain.

## Alternatives

- `include/` + `src/` split — rejected: overkill for an application, not a library

## Consequences

\+ Open any module folder — both files are there  
\- If the project ever turns into a library, header separation becomes extra churn