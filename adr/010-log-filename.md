# ADR 010: Log filename timestamp precision (minutes, not seconds)

**Date:** 2026-08-15
**Deciders:** Tetiana

---

## Context
`Logger::InitLogFile()` names each log file using a timestamp that includes
only hours and minutes, not seconds. Within the same minute, multiple test
runs (`make test`) will overwrite the same log file instead of each getting
its own.

## Decision
Keep minute-only precision for now. At this stage of the project there is no
event loop running yet, no CI, and no automated/looped test runs — tests are
triggered manually, one at a time, with the result checked right after each
run. The risk of two runs landing in the same minute is low, and adding
second-level precision now would be solving a problem that doesn't exist yet.

## Alternatives
- Add seconds to the filename now — rejected: no current need.

## Consequences
+ Simpler timestamp formatting
+ Filenames stay short and readable
- If multiple `make test` runs happen within the same minute (e.g. once CI or
  a watch-script exists), logs silently overwrite each other and visibility
  into a specific run is lost
- Revisit if: CI is added, a watch/auto-rerun script is added, or event loop
  testing starts producing multiple runs per minute