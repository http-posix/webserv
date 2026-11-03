POLLIN - the flag that is typically used nowadays to signal that data has arrived. On listener we only read it in revents (new connection to accept()). On Connection we set it ourselves in events to control the flow, and read it in revents.

POLLRDNORM - a subset of POLLIN, always comes with it, we have no reason to track it separately.

POLLRDBAND/POLLPRI - out of project scope  

POLLRDBAND/POLLPRI - out of project scope  

POLLOUT - set in events when we are ready to send() data back; in revents it means there is room in the buffer for our data  

POLLWRNORM - the same as POLLOUT, no reason to track it separately  

POLLWRBAND - we don't use any priority data, so it's out of scope  

Revents only:
POLLERR -  means errors on fd (like read/write fails) - listener - shutdown, connection - close connection  

POLLHUP - hangup. Never comes with POLLOUT, but can come with POLLIN (data still in buffer). Connection was reset by peer (e.g. client killed with unread data). Tricky if CGI is still running. On listener - not expected

POLLNVAL - fd is not open (already closed, but still in poll set). Bug in our code, not a normal case. Remove from poll set. On listener - not expected


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
