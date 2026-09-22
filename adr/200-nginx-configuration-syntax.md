# ADR 200: nginx-style configuration syntax

**Date:** 2026-09-03
**Recorded:** 2026-22-03 <!-- only if written later -->
**Deciders:** Zach <!-- who made this call -->

---

## Context
In order to set up customizing configuration for different servers and locations;
keywords, syntax and rulesets had to be defined in order to 
parse and validate (in)correct configurations.

## Decision
nginx-style grammar, using `{` and `}` as structural symbols
allowed for a recursive-descent reader consuming `server` and `location`
blocks to specify custom settings for different servers/locations.

Similarly `;` was used as a statement terminator.

## Consequences
Using `{`, `}` and `;` allows for readability for anyone slightly familiar
with compiled programming languages.
<!-- \+ easier -->
<!-- \- trade-off accepted -->
