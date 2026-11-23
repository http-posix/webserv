`curl -v "http://127.0.0.1:8080/cgi-bin/sleep.py"`
ctrl+c => no zombie, connection closed, but script is still working and send data into nowhere

```bash
14:00:35.149 [--INFO---] run_app.cpp:44:      Using configuration from: config/cgi
14:00:35.150 [--INFO---] server.cpp:38:       Server built for: 127.0.0.1:8080
14:00:35.150 [--INFO---] server.cpp:38:       Server built for: 127.0.0.1:8081
14:00:38.953 [--DEBUG--] event_loop.cpp:203: [127.0.0.1:8080 fd=4] accept: new client fd=6 from 127.0.0.1
14:00:38.953 [--DEBUG--] event_loop.cpp:229: [127.0.0.1:8080 fd=4] We are going to OnReadable fd=6 revents=POLLIN
14:00:38.953 [--DEBUG--] connection.cpp:84:  [127.0.0.1:8080 fd=4] reading: recv fd=6 recv_bytes=93
14:00:38.954 [--DEBUG--] connection.cpp:241: [127.0.0.1:8080 fd=4] reading: cgi request parsed -> cgi fd=6 pid=4032
14:00:49.012 [--DEBUG--] event_loop.cpp:223: [127.0.0.1:8080 fd=4] We are going to OnCgi fd=7 revents=POLLIN
14:00:49.019 [--DEBUG--] event_loop.cpp:223: [127.0.0.1:8080 fd=4] We are going to OnCgi fd=7 revents=POLLIN|POLLHUP
14:00:49.019 [--DEBUG--] connection.cpp:268: [127.0.0.1:8080 fd=4] cgi: output ready -> writing pid=4032 fd=6
14:00:49.019 [--DEBUG--] child_reaper.cpp:33:  reap child pid=4032
14:00:49.019 [--DEBUG--] event_loop.cpp:235: [127.0.0.1:8080 fd=4] We are going to OnWritable fd=6 revents=POLLOUT
14:00:49.019 [--DEBUG--] connection.cpp:114: [127.0.0.1:8080 fd=4] writing: send fd=6 send_bytes=70 progress=70/70
14:00:49.019 [--DEBUG--] connection.cpp:117: [127.0.0.1:8080 fd=4] writing: response fully sent, closing fd=6
14:00:49.019 [--DEBUG--] event_loop.cpp:331: [127.0.0.1:8080 fd=4] closed connection fd=6
```