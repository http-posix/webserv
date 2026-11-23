# ADR 108: Race conditions in Unique File creationg.

**Date:** 2026-11-18
**Updated:** 2026-11-23
**Deciders:** Zach

---

## Context
When a post method comes in that does not containt a `content-disposition` header,
we must create a unique filename to store the file as. However, if we have many 
requests coming in we must avoid possible race conditions where we are working with the same file.

## Decision
A simple atomic variable should prevent the possibility from race conditions.

Update 2026-11-23.
A static integer is not needed. We need only to check whether the file exists, and if so, increment the identifier (untill we hit the limit).

## Alternatives
- Using a timestamp as a file identifier. however this creates a possible TOCTOU (time of check, time of use)
    where the filename gets created prior to the writing meaning that checking and using are out of sync which is a security risk
- using a static variable; creates race conditions.
- using `mkstemp()` to generate a unique temporary filename. -> requires ::read() and ::write()

## Consequences
+ Unique and safe filenames.
- Unsure whether `mkstemp` will hold under stress.
