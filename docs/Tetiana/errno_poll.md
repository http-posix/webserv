
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

