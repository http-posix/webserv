# ADR 202: Configuration Structures for Location and Server Settings

**Date:** 2026-09-03
**Recorded:** 2026-09-25
**Deciders:** Tetiana, Zach

---

## Context
Once the configuration file has been tokenized, it must be parsed into data that the program can use.

Specific servers (and their corresponding specific location settings) need to know which variables are (un)set,
leading to the requirement of a structure that can be passed through the program to check for certain variables.

## Decision
A simple config structure for `server` and `location` settings was used. Functions like `createLocationConfig` and
`createServerConfig` directly create and fill the structure during parsing. Semantics and syntaxing happens during
the creation of these config structures.

## Alternatives
- An Abstract Syntax Tree building semantics into the structure itself.
- Nginx-like configuration modules and corresponding registries.

## Consequences
+ Creation and usage of `ServerConfig` and `LocationConfig` is as simple as can be. Pointers can be
passed through as arguments to allow access to check specific variables for settings.
+ Structures are easily copied.
- Config Structures "leak" information to functions that do not use it.
- Future changes/alterations/additions of the simulated AST in `ConfigParser` is difficult to implement as it requires
to read through the entire parser.
