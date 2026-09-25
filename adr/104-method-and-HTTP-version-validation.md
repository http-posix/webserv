# ADR 104: Method and HTTP-version validation.

**Date:** 2026-06-XX
**Recorded:** 2026-09-22
**Deciders:** Zach

---

## Context
The HyperTextTransferProtocol has had substantial upgrades over time, including the often used `HEAD` method from `HTTP/1.1`.
While the subject defines to take `HTTP/1.0` as the prime focus, most current day browsers use version `HTTP/1.1`.

## Decision
Since most browsers send requests with a `HTTP/1.1` version, we decided to allow access only if
the method used conforms to the limits of version `HTTP/1.0`. This way, modern day browsers
can still access the server while keeping the work-load to a minimum.

## Alternatives
- Accept any semantically valid method. (nginx)
- Add an aditional case for the often used `HEAD` method.

## Consequences
+ Reduces the workload while keeping to the requirements of the subjects.
- `HEAD` is rejected as a method resulting in a `400` error.
