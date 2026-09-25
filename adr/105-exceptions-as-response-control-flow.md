# ADR 105: Exceptions as Response Control Flow

**Date:** 2026-09-18
**Recorded:** 2026-09-22
**Deciders:** Zach

---

## Context
Building a response involves many different status codes to inform the client what type of response has been made. 
HTTP defines separation between the following respose codes;

- 1xx: Informational
- 2xx: Success
- 3xx: Redirect
- 4xx: Client Error
- 5xx: Server error

A succesful request goes through the entire process without anything going wrong. The requested file exists, has access,
has the correct method etc. However specific settings or errors might prevent the client from a succesful response. This
means that the response generation must alter its procedure and inform the client with a specific (altered) response message.

## Decision
Through the use of `HttpResponseExceptions` non-succesful requests can be caught and handled. Creating unique response messages
and sending them back through to the user. Functions like `SetStatusMessage()` and enums `ResponseStatusCode` allow the
catching of non-succesfull response status codes and create the correct message through a simple switch-case statement.

Future Statuscodes can be added by creating an additional `enum` and `case` for the `SetStatusMessage()` handler.

## Alternatives
- Early returning and nesting of functions `(if tryLocation() != 0)`. Very C-Style programming.
- Usage of Global variables.

## Consequences
+ Adding new Status codes only requires an additional enum and a case specific handler.
- Ambiguous use of `exceptions`, normally these are for catching errors.

