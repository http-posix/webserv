# Listener ownership 

## Ownership

EventLoop owns three containers:

```
listeners_     vector<Server>          one per unique host:port
connections_   map<int, Connection>    keyed by client fd
poll_fds_      vector<pollfd>          listeners + clients + cgi pipes
```

Ownership chain:

```
int fd  ->  Socket (RAII)  ->  Server  ->  EventLoop
```

Server holds the listening Socket and runs bind/listen/accept. EventLoop never
touches a raw socket; on a listener event it asks the matching Server to accept.


## Config never enters EventLoop

Config is read outside the loop. A builder turns Config into a ready
vector<Server> and injects it through the EventLoop constructor. host:port
dedup happens in the builder. EventLoop knows nothing about Config.

Result: one reason-to-change per class.

```
Server       socket creation logic
EventLoop    I/O model (poll)
Connection   client states
```

If EventLoop read Config and built Servers itself, it would gain a second
reason to change (config format). Building listeners outside avoids that.

## Build sequence (pseudocode)

```
build_listeners(config):
    seen      = {}            # set of host:port
    listeners = []
    for server_block in config.servers:
        for (host, port) in server_block.listen:
            if (host, port) in seen:
                continue       # dedup, otherwise second bind -> EADDRINUSE
            seen.add(host, port)
            srv = Server(host, port)     # socket -> setsockopt(REUSEADDR) -> bind -> listen
            listeners.push_back(move(srv))
    return listeners

main():
    config    = make_stub_config()       # later: parse_config(path)
    listeners = build_listeners(config)
    loop      = EventLoop(move(listeners))
    loop.run()
```

The only place that touches Config and does dedup is build_listeners. EventLoop
receives finished objects.

## Note: move-only

Server holds a move-only Socket, so Server is move-only too. Its move
constructor must be noexcept, otherwise vector<Server> falls back to copy on
reallocation (which does not exist) instead of moving. Same rule already applied
to Socket.# Listener ownership & config injection

Design notes for the networking core. Covers who owns the listening sockets and
why Config is kept out of the event loop.

## Ownership

EventLoop owns three containers:

```
listeners_     vector<Server>          one per unique host:port
connections_   map<int, Connection>    keyed by client fd
poll_fds_      vector<pollfd>          listeners + clients + cgi pipes
```

Ownership chain:

```
int fd  ->  Socket (RAII)  ->  Server  ->  EventLoop
```

Server holds the listening Socket and runs bind/listen/accept. EventLoop never
touches a raw socket; on a listener event it asks the matching Server to accept.

There are several listeners, not one, because the config can declare multiple
interface:port pairs. So it is a vector, not a single field.

## Config never enters EventLoop

Config is read outside the loop. A builder turns Config into a ready
vector<Server> and injects it through the EventLoop constructor. host:port
dedup happens in the builder. EventLoop knows nothing about Config.

Result: one reason-to-change per class.

```
Server       socket creation logic
EventLoop    I/O model (poll -> epoll)
Connection   client states
```

If EventLoop read Config and built Servers itself, it would gain a second
reason to change (config format). Building listeners outside avoids that.

## Build sequence (pseudocode)

```
build_listeners(config):
    seen      = {}            # set of host:port
    listeners = []
    for server_block in config.servers:
        for (host, port) in server_block.listen:
            if (host, port) in seen:
                continue       # dedup, otherwise second bind -> EADDRINUSE
            seen.add(host, port)
            srv = Server(host, port)     # socket -> setsockopt(REUSEADDR) -> bind -> listen
            listeners.push_back(move(srv))
    return listeners

main():
    config    = make_stub_config()       # later: parse_config(path)
    listeners = build_listeners(config)
    loop      = EventLoop(move(listeners))
    loop.run()
```

The only place that touches Config and does dedup is build_listeners. EventLoop
receives finished objects.

