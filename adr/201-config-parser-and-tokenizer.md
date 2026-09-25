# ADR 201: Splitting config reader into tokenizer and parser

**Date:** 2026-09-03 
**Recorded:** 2026-09-22  <!-- only if written later -->
**Deciders:** Zach<!-- who made this call -->

---

## Context
Reading a text file into usable configuration requires two very different kinds
of work: turning characters into meaningful units (words, numbers, symbols) and
turning those units into validated settings. If both are smeared into one class,
the code is hard to test, hard to reason about, and every grammar change touches
both concerns.
<!-- what situation forced this decision, 1-3 sentences -->

## Decision
Splitting the config reader into a `tokenizer` and a `parser`/`validator`.

The tokenizer is a *dumb* cursor and mainly recognizes words as a *type* of word (token),
while not knowing the meaning of the word. The tokenizer can iterate over the entire 
configuration file with a `next()` method and check what kind of a word it is on.

The parser can then easily understand the meaning of the configuration by looking at 
the sequence of tokens. By using a tokenizer, rulesets can easily be created without 
having to "*read*" every word.

As an Example:
```
if (type == keyWord && type.Next() == value)
    // Initialize the value for the specific keyword.
```

## Alternatives
A single class that both scans and builds a config. Messy and difficult to test in
isolation.
<!-- - option — why not (skip if none worth mentioning) -->

## Consequences
+ Each piece is testable in isolation using unit tests.
- Two classes means more work for future implementations.
<!-- \+ easier -->
<!-- \- trade-off accepted -->
