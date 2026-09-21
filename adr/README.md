# Architecture Decision Records

This folder records the key technical decisions made in this project,
and the reasoning behind them.

## Numbering

| Range     | Area                                        |
|-----------|---------------------------------------------|
| `000–099` | Engine (event loop, connections, CGI execution) |
| `100–199` | HTTP (request parser, request, response)    |
| `200–299` | Configuration file & config parser          |

## Adding an ADR

1. Copy `adr-template.md`.
2. Name it `NNN-short-title.md`, using the next free number in the matching range.
3. If the decision was made earlier than it was written down, set `Date` to the decision date and add `Recorded`.