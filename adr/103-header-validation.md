# ADR 103: HTTP-Header validation

**Date:** 2026-06-XX
**Recorded:** 2026-09-22
**Deciders:** Zach

---

## Context
Headers are a grey area for malicious users, possible injecting CariageLineFeed (`\r\n`) into headers to break
applications. RFC 7230 has very strict rules from preventing invalid charactes into headers. A decision must 
be made whether to implement the same strictness or, like nginx, be more on the lenient side.

## Decision
Since the subject defines a `segmentation fault` as an immediate fail, it is better to be safe than sorry.
Thus the decision was made to follow the RFC7230 guideline as much as possible.

## Alternatives
nginx does not enfore the full FC7230 guideline, allowing for excess whitespace in headers and 
odd but not-necessarily-breaking input.

## Consequences
+ Safe; we prefer to reject possibly ambiguis input rather than remain lenient.
- Clients might receive a `400` response due to folded or invalid headers. Nginx handles this more defined.

