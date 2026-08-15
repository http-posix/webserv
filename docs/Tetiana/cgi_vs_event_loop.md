- CGI-Specific: Pipe Tracking
CGI will fork + exec and need pipes back in the loop:

- I must add these pipe fds into poll() array. CGI output comes back async — loop reads it just like any other fd and appends to ClientConnection.write_buffer.

- How does CGI signal completion? Sets response_ready directly, or calls a callback into your loop?