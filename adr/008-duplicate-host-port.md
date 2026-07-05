# ADR 008: Duplicate host:port is a config error

**Date:** 2026-06-20
**Deciders:** Tetiana

---

## Context

`CreateListeners` builds one `Server` (one listening socket) per unique `host:port` pair found across the whole config. This project does not support virtual hosts. So there is no legitimate reason for two different config blocks to request the same `host:port`.

## Decision

If `CreateListeners` finds a `host:port` pair that was already seen earlier in the same config, it throws a `ServerException` immediately.

## Alternatives

- Silently skip duplicates and keep only the first `Server` for that pair — rejected. Users wouldn't know their config was partially ignored. Since virtual hosts aren't supported, a duplicate host:port is likely a configuration error that should be reported.
- Let the second `bind()` fail naturally and report the resulting `EADDRINUSE` — rejected: this looks like a random runtime/system failure rather than a clear statement about what's actually wrong with the config.

## Consequences

+ Configuration errors are caught early with error messages, before any socket is created. (Host:port details are available in debug mode.)