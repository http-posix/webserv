
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

---

POLLIN  
The file descriptor is ready for reading data other than high-priority data.  

POLLRDNORM  
The file descriptor is ready for reading normal data.  

POLLRDBAND  
The file descriptor is ready for reading priority data.  

POLLPRI  
The file descriptor is ready for reading high-priority data.  

POLLOUT  
The file descriptor is ready for writing normal data.  

POLLWRNORM  
Equivalent to POLLOUT.  

POLLWRBAND  
The file descriptor is ready for writing priority data.  

POLLERR  
An error condition is present on the file descriptor. All error conditions that arise solely from the state of the object underlying the open file description and would be diagnosed by a return of -1 from a read() or write() call on the file descriptor shall be reported as a POLLERR event. This flag is only valid in the revents bitmask; it shall be ignored in the events member.  

POLLHUP  
A device has been disconnected, or a pipe or FIFO has been closed by the last process that had it open for writing. Once set, the hangup state of a FIFO shall persist until some process opens the FIFO for writing or until all read-only file descriptors for the FIFO are closed. This event and POLLOUT are mutually-exclusive. However, this event and POLLIN, POLLRDNORM, POLLRDBAND, or POLLPRI are not mutually-exclusive. This flag is only valid in the revents bitmask; it shall be ignored in the events member.  

POLLNVAL  
The specified fd value is not an open file descriptor. This flag is only valid in the revents member; it shall be ignored in the events member.
