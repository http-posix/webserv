type punning via a common prefix
prefix-based type punning == common-prefix type punning
for IPv4 & IPv6 struct starts from the same field sa_family_t sa_family

https://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/socket.h.html

```
void AcceptConnection(int fd, const Server& l){
	sockaddr_in		addr{};
	socklen_t		addr_len = sizeof(addr);

	accepted_fd = ::accept(fd, reinterpret_cast<sockaddr*>(&addr), &addr_len);
	
}
```