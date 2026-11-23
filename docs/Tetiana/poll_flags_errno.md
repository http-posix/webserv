## flags

POLLIN - the flag that is typically used nowadays to signal that data has arrived. 
- listener: we only read it in revents (new connection to accept()).
- connection: we set it ourselves in events to control the flow, and read it in revents.
- cgi read pipe (script stdout): there is output to read().


POLLRDNORM - a subset of POLLIN, always comes with it, no reason to track it separately.

POLLRDBAND/POLLPRI - out of project scope  

POLLOUT - set in events when we are ready to send() data back; in revents it means there is room in the buffer for our data.  

POLLWRNORM - the same as POLLOUT, no reason to track it separately  

POLLWRBAND - we don't use any priority data, so it's out of scope  

Revents only:
POLLERR -  means errors on fd (like read/write fails)  
- listener: not expected (only if somebody broke the socket by hand) -> shutdown  
- connection: usually comes together with POLLIN/POLLOUT, so the normal handler runs, recv()/send() returns -1 -> close connection  
- cgi write pipe: the script closed stdin before reading the whole body. Normal, not an error -> close only the write end, the script keeps running and we read its output  

POLLHUP - hangup.  
- listener: not expected -> shutdown.
- connection: both directions are closed. Reset by peer gives POLLERR|POLLHUP. Usually comes with POLLIN, recv() returns 0/-1 -> close connection.
  POSIX says it never comes with POLLOUT, but on Linux TCP it can (on macOS - no).
  If the client is gone while CGI is running -> close connection and kill the CGI child.
- cgi read pipe: normal EOF, the script closed stdout / exited. Not an error. Can come with or without POLLIN -> read() returns 0 -> build the response.

POLLNVAL - fd is not open (already closed, but still in poll set). Bug in our code, not a normal case. 
- listener -> shutdown.
- connection socket -> LOG_ERROR + close connection.
- cgi pipe -> LOG_ERROR, then the normal handler: read() = -1 -> 502, write() = -1 -> close the write end.



**POLLIN**  
The file descriptor is ready for reading data other than high-priority data.  

**POLLRDNORM**  
The file descriptor is ready for reading normal data.  

**POLLRDBAND** - we have't this case in Webserv  
The file descriptor is ready for reading priority data.  

**POLLPRI** - we have't this case in Webserv  
The file descriptor is ready for reading high-priority data.  

**POLLOUT**  
The file descriptor is ready for writing normal data.  

**POLLWRNORM**  
Equivalent to POLLOUT.  

**POLLWRBAND**  
The file descriptor is ready for writing priority data.  

**POLLERR**  
An error condition is present on the file descriptor. All error conditions that arise solely from the state of the object underlying the open file description and would be diagnosed by a return of -1 from a read() or write() call on the file descriptor shall be reported as a POLLERR event. This flag is only valid in the revents bitmask; it shall be ignored in the events member.  

**POLLHUP**  
A device has been disconnected, or a pipe or FIFO has been closed by the last process that had it open for writing. Once set, the hangup state of a FIFO shall persist until some process opens the FIFO for writing or until all read-only file descriptors for the FIFO are closed. This event and POLLOUT are mutually-exclusive. However, this event and POLLIN, POLLRDNORM, POLLRDBAND, or POLLPRI are not mutually-exclusive. This flag is only valid in the revents bitmask; it shall be ignored in the events member.  

**POLLNVAL**  
The specified fd value is not an open file descriptor. This flag is only valid in the revents member; it shall be ignored in the events member.  

## errno 

ERRORS  
The poll() and ppoll() functions shall fail if:

[EAGAIN]  
The allocation of internal data structures failed but a subsequent request may succeed.  

[EINTR]  
A signal was caught during poll() or ppoll().  

[EINVAL]  
The nfds argument is greater than {OPEN_MAX}.  
The ppoll() function shall fail if:  

[EINVAL]  
An invalid timeout interval was specified.  

## accept()
	https://man7.org/linux/man-pages/man2/accept.2.html  

- EMFILE (process fd limit) / ENFILE (system fd limit) -> WARN + pause accepting on all listeners.  
  Without pause it's a busy loop: the client waits in backlog, listener stays readable, poll returns at once.  
  Resume when an fd is freed (close connection, StopWatching a cgi pipe) or when there are no connections at all (ENFILE).  
- EAGAIN / EWOULDBLOCK, ECONNABORTED, EINTR -> not errors: the client left before we accepted it, or a signal came. Just return, poll() will tell us again if something is waiting.  
- anything else -> DEBUG log, return.  

## errno from poll()

- EINTR - a signal came (for us only SIGINT/SIGTERM) -> DEBUG, try again. while (g_signal_running) decides if we stop.
- EAGAIN (POSIX, macOS) / ENOMEM (Linux) - kernel has no memory for now -> WARN, try again.
- EINVAL - nfds is bigger than the fd limit, means duplicates / dead fds in our vector. Our bug -> exception.
- EFAULT - bad pointer to the array. With std::vector only if memory is broken. Our bug -> exception.

EINVAL / EFAULT are our bugs, not the OS: trying again with the same arguments fails the same way.