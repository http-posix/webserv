whhat happens if we send POST-script with wrong method GET

```bash
~/Codam/02_new_webserv> ./webserv config/cgi 
Welcome to Webserv of the http-posix team!
09:41:08.656 [--INFO---] run_app.cpp:44:      Using configuration from: config/cgi
09:41:08.656 [--INFO---] server.cpp:38:       Server built for: 127.0.0.1:8080
09:41:08.656 [--INFO---] server.cpp:38:       Server built for: 127.0.0.1:8081
09:42:51.634 [--DEBUG--] event_loop.cpp:201: [127.0.0.1:8080 fd=4] accept: new client fd=6 from 127.0.0.1
09:42:51.635 [--DEBUG--] event_loop.cpp:227: [127.0.0.1:8080 fd=4] We are going to OnReadable fd=6 revents=POLLIN
09:42:51.635 [--DEBUG--] connection.cpp:84:  [127.0.0.1:8080 fd=4] reading: recv fd=6 recv_bytes=96
09:42:51.637 [--DEBUG--] connection.cpp:242: [127.0.0.1:8080 fd=4] reading: cgi request parsed -> cgi fd=6 pid=49356
dffffg
^CTraceback (most recent call last):
  File "/Users/Amirre/Codam/02_new_webserv/www/cgi-bin/cgi_post.py", line 15, in <module>

 Server stopped. Have a good day ^_^
    main()
  File "/Users/Amirre/Codam/02_new_webserv/www/cgi-bin/cgi_post.py", line 4, in main
    body = sys.stdin.read()
KeyboardInterrupt
```

the same, but after "sssss..s" ctrl+d (EOF) => data was sended into script
```bash
~/Codam/02_new_webserv> ./webserv config/cgi 
Welcome to Webserv of the http-posix team!
09:45:36.732 [--INFO---] run_app.cpp:44:      Using configuration from: config/cgi
09:45:36.733 [--INFO---] server.cpp:38:       Server built for: 127.0.0.1:8080
09:45:36.733 [--INFO---] server.cpp:38:       Server built for: 127.0.0.1:8081
09:45:42.332 [--DEBUG--] event_loop.cpp:201: [127.0.0.1:8080 fd=4] accept: new client fd=6 from 127.0.0.1
09:45:42.333 [--DEBUG--] event_loop.cpp:227: [127.0.0.1:8080 fd=4] We are going to OnReadable fd=6 revents=POLLIN
09:45:42.333 [--DEBUG--] connection.cpp:84:  [127.0.0.1:8080 fd=4] reading: recv fd=6 recv_bytes=96
09:45:42.335 [--DEBUG--] connection.cpp:242: [127.0.0.1:8080 fd=4] reading: cgi request parsed -> cgi fd=6 pid=49483
ssssssssssssssssssssssss09:45:54.879 [--DEBUG--] event_loop.cpp:221: [127.0.0.1:8080 fd=4] We are going to OnCgi fd=7 revents=POLLIN
09:45:54.885 [--DEBUG--] event_loop.cpp:221: [127.0.0.1:8080 fd=4] We are going to OnCgi fd=7 revents=POLLIN|POLLHUP
09:45:54.885 [--DEBUG--] connection.cpp:269: [127.0.0.1:8080 fd=4] cgi: output ready -> writing pid=49483 fd=6
09:45:54.885 [--DEBUG--] event_loop.cpp:233: [127.0.0.1:8080 fd=4] We are going to OnWritable fd=6 revents=POLLOUT
09:45:54.885 [--DEBUG--] connection.cpp:114: [127.0.0.1:8080 fd=4] writing: send fd=6 send_bytes=78 progress=78/78
09:45:54.885 [--DEBUG--] connection.cpp:117: [127.0.0.1:8080 fd=4] writing: response fully sent, closing fd=6
09:45:54.885 [--DEBUG--] event_loop.cpp:321: [127.0.0.1:8080 fd=4] closed connection fd=6
^C
 Server stopped. Have a good day ^_^
```