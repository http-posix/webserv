# ADR 109: Reject POST and DELETE methods by default

**Date:** 2026-11-23
**Deciders:** Zach

---

## Context
In the router, when a location has been found, we check whether the location allows for the requested method. However, when no location is found, we don't have this check.


## Decision
Since DELETE and POST are file altering methods, I decided that the default for these methods is off and that they are only allowed if the server admin specifically allows for these methods (on specific locations!)
<!-- what situation forced this decision, 1-3 sentences -->
<!-- what was decided and why this over alternatives -->

## Alternatives
- Setting the default to be on. And having specific guards for folders that have sensitive information.
<!-- - option — why not (skip if none worth mentioning) -->
