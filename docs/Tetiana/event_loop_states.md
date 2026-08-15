# Event Loop — pseudocode

```

LOOP forever:
    poll(poll_fds, nfds, timeout)

    for each fd in poll_fds where revents != 0:

        if POLLERR or POLLHUP:
            closeClient(fd)
            continue

        if fd is listening socket and POLLIN:
            client_fd = accept()
            fcntl(client_fd, O_NONBLOCK)
            create ClientConnection { fd, state = StateReading, last_activity = now }
            add client_fd to poll_fds watching POLLIN
            add to clients map

        if fd is client fd:
            client = clients[fd]
            std::visit routes to handler based on client.state:

                StateReading + POLLIN:
                    bytes = recv(fd, chunk)
                    if bytes == 0 or error  →  closeClient(fd)
                    resut_status = client.parser.parse(client.read_buffer) //result_status is enum
                    update last_activity
                    if result_status == INCOMPLETE  →  do nothing, wait next POLLIN
                    if result_status == ERROR       →  closeClient(fd)
                    if result_status == COMPLETE:
                        client.http_request = result.request//client.parser.getRequest();
                        if not is_cgi  →  response fills write_buffer
                                          client.state = StateWriting { offset = 0 }
                                          flip fd to POLLOUT
                        if is_cgi      →  cgi forks, returns pipe_fd + pid  
                                          client.state = StateCGI { pipe_fd, pid }
                                          add pipe_fd to poll_fds watching POLLIN

                StateWriting + POLLOUT:
                    sent = send(fd, write_buffer from offset)
                    offset += sent
                    update last_activity
                    if offset == write_buffer.size():
                        if keep_alive  →  client.state = StateReading, flip fd to POLLIN
                        else           →  closeClient(fd)

                StateCGI + POLLIN on pipe_fd:  
                    read(pipe_fd) → append to write_buffer
                    if EOF:
                        waitpid(pid)
                        close(pipe_fd), remove from poll_fds
                        client.state = StateWriting { offset = 0 }
                        flip client fd to POLLOUT

    after iterating all fds:
        for each client in clients map:
            if now - last_activity > timeout  →  closeClient(fd)

closeClient(fd):
    close(fd)
    remove from clients map
    remove from poll_fds  (swap with last, decrement nfds)
    if CGI pipe still open  →  close(pipe_fd), remove from poll_fds
```

## notes

!NB who keep uncompleted buffer - Zach or me?

### CgiProcess (event loop ↔ cgi module)
Since CGI output comes back async, I need to put its pipes into my poll() array.

- stdin_pipe_fd — where I write the POST body for the script.
- stdout_pipe_fd — where I read CGI output.
- pid — for waitpid()
- client_fd — to know which client this CGI belongs to.
- started_at — to kill the script if it hangs.

### States of clients (state macnihe -> make clear encapsulation )

***StateReading***
The struct is completely empty, if  Zach-parser handles accumulation. I just feed it.

***StateWriting***
Carries the offset (size_t). Need to remember how far send() got through the response string, because it might not send everything in one go. Next POLLOUT, I resume from this offset.

***StateCGI***
Carries pipe_fd and pid. The client fd just sits idle while we wait for the pipe. 


If POLLERR or POLLHUP:
Client disconnected, remove the fd from the array and map, and continue.

If POLLIN on a listening socket:
- accept() the new client.
- fcntl() to non-blocking.
- create ClientConnection, set state to StateReading.
- update last_activity.
- add to poll array watching POLLIN.
- put it in the map.

If POLLIN or POLLOUT on a client fd:
Find the connection in the map, then let std::visit route it based on the current state.
- StateReading + POLLIN -> handleRead()
- StateWriting + POLLOUT -> handleWrite()
- StateCGI + POLLIN on pipe -> handleCGI()

### Event handlers

handleRead()
- call recv() into a local array – am I?
- if 0: client disconnected normally, close client.
- if -1 and EAGAIN: nothing to read, just return.
- otherwise: pass the local chunk directly to Zach (or storing on my side?)
- update last_activity.

If parser returns COMPLETE, check if it's CGI:
- if not CGI: response fills their buffer. I set state to StateWriting{0} and flip fd to POLLOUT.
- if CGI: CGI Jo handles fork. I get pipe_fd and pid, set state to StateCGI, and watch pipe_fd for POLLIN.

handleWrite()
- call send() using the response bytes and my current offset.
- add the return value of send() to my offset.
- if offset == total response size: we are done.
  - if keep_alive is true: reset state to StateReading, destroy offset, flip fd back to POLLIN.
  - if false: closeClient().
- update last_activity.

handleCGI()
- read() from pipe_fd.
- if 0: pipe closed. call waitpid(). remove pipe_fd from poll and close it. set state to StateWriting{0} and flip client fd to POLLOUT.



