# ADR 102: Functioning loop of HTTP parser

**Date:** 2026-06-XX
**Recorded:** 2026-09-22
**Deciders:** Zach

---

## Context
Since the desicion to make the Parser a Finite State Machine, the parser has different
states in which it parser different material. However information through the socket can
come in all at once. Thus the Parser needs to both be able to handle incomplete requests
and complete requests.

## Decision
Every State of the parser (`ParseRequestLine()`, `ParseHeaders()` and `ParseBody()`) returns a simple
`0` or `-1` identifying whether the specific state is complete. 

Whenever a state is complete, the `internal_state` variable is set to the next case (untill completion).

If at any point the states runs out of information without completing its state, it returns `-1` to 
to break out of the while loop.

```
	while (loop == 0)
	{
		switch (state_)
		{
			case Error :
				loop = -1;
				break ;
			case RequestLine :
				loop = ParseRequestLine();
				break ;
			case Header :
				loop = ParseHeaders();
				break ;
			case Body :
				loop = ParseBody();
				break ;
			case Done :
				loop = -1;
				break ;
		}
	}
	return (external_state_);
```

## Alternatives

## Consequences
+ Compact Control Flow. The `while` loop covers all stages.
+ Clear definition. while `loop == 0` the parser still has work to do.
- Possible ambiguity between `external_state_` and `internal_state_`. 
`internal_state` is for the parser while `external_state_` is for communication for the event_loop.

