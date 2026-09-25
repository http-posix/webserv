# ADR 203: Client Max Body Size Parsing

**Date:** 2026-09-03
**Recorded:** 2026-25-09
**Deciders:** Zach

---

## Context
Part of the subject requires the ability to set the `client_max_body_size`. Since computers
work in bytes only (which is hard to read for humans), the need for defining units arised.
(kilobyte, megabyte, gigabyte).

## Decision
The Config parser has a helper function called `applyUnit()` which checks the unit of
the given number. The `applyUnit()` function then converts the unit into actual bytes
with data type `size_t`. If no unit is found, the raw number is used as bytes.

Thus if the user sets the `client_max_body_size` to `2kb`, `applyUnit()` will convert this
to `2*1024` bytes. (`1024^2` for `mb` etc)

## Alternatives
No implementation of units whatsoever. The user has to specify the raw bytes only.

## Consequences
+ Usability for the user is much more friendly. The computer converts the unit into actual bytes.
+ If no unit is given, the value is still valid (default is in bytes).
- Careful handling of the ruleset for `max_client_body_size` in the `ConfigParser`.
