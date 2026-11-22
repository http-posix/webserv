1. Early return to avoid nesting
2. Explicitness is better than implicitness
3. DRY - Don't repeat yourself – avoid code duplication
4. SRP - Single Responsibility
5. Imperative Mood – write commit messages in the imperative mood, which gives a sense of direction for the change. Start the subject with a verb, such as "Fix," "Add," or "Update."
6. Make a function a member only if it needs direct access to the representation of a class
7. Document each class / free function in the header: its responsibility and contract (ownership, invariants, pre/postconditions), not a restatement of its name or how it's implemented.
8. You need to have a system for how you track errors, the same logic to have consistency
9. Const-correctness – if a method could be const it should be const
10. Check for double close / closing invalid fds:
```bash
    strace -f -e trace=close ./webserv config/default.conf 2>&1 | grep EBADF
```
`-f` - also trace child processes (CGI after fork)  
`-e trace=close` - show only close() calls  
`2>&1` - strace writes to stderr, redirect it so grep can see it  
`grep EBADF` - keep only failed closes, e.g. `close(5) = -1 EBADF (Bad file descriptor)` -> fd was already closed or never opened  
   or  
```bash
   strace -f -e trace=close -o trace.log ./webserv config/default.conf
   # after ctrl-c or in other terminal
   grep EBADF trace.log
```



By value vs by reference:
trivially copyable data and sizeof at most (2 * sizeof(void*)) - pass by value, no const - size of void* depends on the system (Raspberry Pi or Arduino have a different size)
```cpp
	static_assert(std::is_trivially_copyable_v<pollfd>);
	static_assert(sizeof(pollfd) <= 16);
```

otherwise: by reference + const by default, without const only when you give permission to change original data.  
Sources:
- [cppreference: is_trivially_copyable](https://en.cppreference.com/w/cpp/types/is_trivially_copyable)
- [C++ Core Guidelines: F.16: For “in” parameters, pass cheaply-copied types by value and others by reference to const](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#rf-in) - check **Enforcement**
- [Tip of the Week #109: Meaningful `const` in Function Declarations](https://abseil.io/tips/109)
- [Tip of the Week #234: Pass by Value, by Pointer, or by Reference?](https://abseil.io/tips/234)


## IPv4 vs IPv6
type punning via a common prefix
prefix-based type punning == common-prefix type punning
for IPv4 & IPv6 struct starts from the same field sa_family_t sa_family


https://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/socket.h.html

```
void AcceptConnection(int fd, const Server& l){
	sockaddr_in		addr{};
	socklen_t		addr_len = sizeof(addr);

	int accepted_fd = ::accept(fd, reinterpret_cast<sockaddr*>(&addr), &addr_len);
	
}
```

Note: sockaddr_in holds only an IPv4 address. To support both IPv4 and IPv6,
use sockaddr_storage (big enough for any address type) and check ss_family:

```cpp
sockaddr_storage	addr{};
socklen_t			addr_len = sizeof(addr);
int					accepted_fd = ::accept(fd, reinterpret_cast<sockaddr*>(&addr), &addr_len);

if (addr.ss_family == AF_INET)        // IPv4
	auto* v4 = reinterpret_cast<sockaddr_in*>(&addr);
else if (addr.ss_family == AF_INET6)  // IPv6
	auto* v6 = reinterpret_cast<sockaddr_in6*>(&addr);
```

## Signals

**SIGINT**  
 `ctrl-c`
 `kill -INT <pid>`

**SIGTERM**  
`kill <pid>`
`killall webserv` / `pkill webserv`
e.g. docker stop will send SIGTERM


check processes
```bash
ps                      # only processes of the current terminal
ps -u $USER             # all processes of your user (no system ones)
pgrep -l webserv        # find by name -> pid + name
pgrep -P <pid>          # children of a process (handy for CGI)
```

for cgi check
```bash
kill <server_pid>
ps -o pid,ppid,stat,comm -u $USER
```

check ports
```
lsof -i :8080           # who is holding port 8080
```

```bash
echo $? # run right after the server stops
```
- `130` = 128 + 2 (SIGINT) -> killed by the signal itself, no handler
- `0` / other -> signal was handled, program exited on its own


`strace -f ./webserv` - Linux, not MacOS
