# ADR 204: Handling of Invalid Configuration Files.

**Date:** 2026-09-03
**Recorded:** 2026-09-25
**Deciders:** Zach

---

## Context
Errors in the configuration should never occur at request time. An out-of-range port or
an invalid `error_page` should not be discovered at run time and must be caught prior
of execution of the `event_loop`.

The question becomes how these errors are reported and how the program reacts.

## Decision
Through the `ConfigException`, errors are caught when initializing/parsing the Configuration structure.

The `ConfigParser` defines strict rulesets for variables, using the `tokenizer` for expected input. 
If any of the rulesets are violated the `parseConfig()` function will throw an exception
specifying the violated rule, what it expected and what it found. 

Every detectable problem (unterminated blocks, empty config file, missing values, invalid response/error codes, etc)
is caught and handled withing the `ConfigParser` object. Without a valid configuration the application will
not start the `event_loop`.

This allows for easier problem solving when invalid configuration files are given and prevents
the application from executing on faulty configurations.

## Alternatives
- nginx permits specific invalid settings, warning the user but not preventing further execution.

## Consequences
+ Invalid configurations are handled before start up, not during the `event_loop`.
+ Prevents possible crashes due to invalid configuration files.
+ Specific error messages allow the user to identify the problem in the config file.
- Configuration syntax is very strict, requiring the user proper reading begore writing their own config.

