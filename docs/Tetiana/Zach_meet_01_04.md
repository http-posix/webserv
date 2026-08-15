
## Two approaches with keep buffer:
### Tetiana own the buffer, he just parses
Tetiana:  client.read_buffer += chunk  
Zach:  parser.parse(client.fd, client.read_buffer)  
Zach:  just reads the buffer, returns INCOMPLETE or COMPLETE + HttpRequest no internal map needed  

### he owns the buffer, Tetiana just feed chunks
Tetiana:  parser.feed(client.fd, chunk, bytes)  
Zach:  has internal map fd → partial state, accumulates himself returns INCOMPLETE or COMPLETE + HttpRequest  
  
Both are clean. The question is just where the buffer lives.  

**First approach** is simpler to reason about — everything about a client is in one place, your ClientConnection. Teammate's code is stateless, easier to test in isolation.

**Second approach** is cleaner separation — Zach owns everything related to parsing including the buffer, Tetiana own everything related to I/O.

## about status 
	I need enums that track status of Zach process

