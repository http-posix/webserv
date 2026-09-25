# ADR 107: Unrecognized content Type Handling.

**Date:** 2026-09-11
**Recorded:** 2026-09-22
**Deciders:** Zach

---

## Context
Information from the server comes in bytes. A browser reads everything as bytes turning them into `char` untill the body
is seen. Not all content received needs to be `char`. A server can also send binary or other types of data. A browser
know what type of data it is handed by the `Content-Type` header. However, if a server sends data that itself does
not recognize, the browser does not know what how to display the information to the user.

## Decision
The `DetermineContentType()` sets the specific header for the response message. It looks at the extension of the
requested file and looks it up in a map of recognized data types. When the extension is recognized, the correct
data type is set in the `Content-Type` header. When an unrecognized content-type is encountered the server sends
back a `415 Unsupported Media Type` error, indicating that the server does not support (or accept) the requested
file type.

## Alternatives
Nginx handles unrecognized data types as `plain/text`. Being lenient and still returning the client their requested resource.

## Consequences
+ Meaningful error. `415` is an HTTP defined response status that informs the user their requested resource is not supported
by the server.
- Limits the application in responses.
- Adding new data types requires altering of the `DetermineContentType()` function.
