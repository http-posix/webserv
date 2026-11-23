call:
```bash
~> curl -v "http://127.0.0.1:8080/cgi-bin/hello.py"
```

```bash
19:03:32.315 [--INFO---] run_app.cpp:44:      Using configuration from: config/cgi
19:03:32.316 [--INFO---] server.cpp:38:       Server built for: 127.0.0.1:8080
19:03:32.316 [--INFO---] server.cpp:38:       Server built for: 127.0.0.1:8081
19:03:47.070 [--DEBUG--] event_loop.cpp:198: [127.0.0.1:8080 fd=4] accept: new client fd=6 from 127.0.0.1
19:03:47.070 [--DEBUG--] event_loop.cpp:224: [127.0.0.1:8080 fd=4] We are going to OnReadable fd=6 revents=POLLIN
19:03:47.070 [--DEBUG--] connection.cpp:79:  [127.0.0.1:8080 fd=4] reading: recv fd=6 recv_bytes=93
19:03:47.071 [--DEBUG--] connection.cpp:219: [127.0.0.1:8080 fd=4] reading: cgi request parsed -> cgi fd=6 pid=23489
19:03:47.131 [--DEBUG--] event_loop.cpp:218: [127.0.0.1:8080 fd=4] We are going to OnCgi fd=7 revents=POLLIN
19:03:47.133 [--DEBUG--] event_loop.cpp:218: [127.0.0.1:8080 fd=4] We are going to OnCgi fd=7 revents=POLLIN|POLLHUP
19:03:47.133 [--DEBUG--] connection.cpp:246: [127.0.0.1:8080 fd=4] cgi: output ready -> writing pid=23489 fd=6
19:03:47.133 [--DEBUG--] event_loop.cpp:230: [127.0.0.1:8080 fd=4] We are going to OnWritable fd=6 revents=POLLOUT
19:03:47.133 [--DEBUG--] connection.cpp:109: [127.0.0.1:8080 fd=4] writing: send fd=6 send_bytes=76 progress=76/76
19:03:47.133 [--DEBUG--] connection.cpp:112: [127.0.0.1:8080 fd=4] writing: response fully sent, closing fd=6
19:03:47.133 [--DEBUG--] event_loop.cpp:298: [127.0.0.1:8080 fd=4] closed connection fd=6
```
