# ADR 106: match locations by longest prefix with boundary rules

**Date:** 2026-09-19
**Recorded:** 2026-09-22
**Deciders:** Zach

---

## Context
A request path must be checked against every location set by the ServerConfiguration.
This could mean that there is a specific location set for `/example1/` while also
having a location block set for `/example1/sub_example2/`. If a request comes through
with `GET /example1/sub_example2/some_image.png` the server must check which location
block has the longest prefix match for the request.

## Decision
For every request, the application goes through all location blocks and returns
the longest possible match.
```c++
if (loc_len > longest_match || result == NULL)
{
    longest_match = loc_len;
    result = &locations[i];
}
```

## Alternatives
- Regex build parsing. Split everything into (sub)directories and build location paths as nested.

## Consequences
+ Works without implementing RegEx (RegEx can seem quite strange if never encountered before)
- Possible future adaptation might become hard to implement.
