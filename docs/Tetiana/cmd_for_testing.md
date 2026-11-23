`curl -v "http://localhost:8080/cgi-bin/hello.py/extra/path?name=value"`  

**Reading `-v` output:**
- `>` is what curl sent to the socket (the request)
- `<` is what the server returned (status and response headers)
- `*` is curl's own info (connecting, connection reuse, etc.)

## Basic GET

```bash
curl http://localhost:8080/                    # response body only
curl -i http://localhost:8080/                 # + status line and response headers
curl -v http://localhost:8080/                 # full request/response exchange
curl -s -o /dev/null -w "%{http_code}\n" http://localhost:8080/   # status code only
```

## Methods

```bash
curl -X POST -d "name=value" http://localhost:8080/upload        # POST with a body
curl -X POST --data-binary @file.txt http://localhost:8080/upload # POST a file as-is
curl -F "file=@photo.png" http://localhost:8080/upload            # multipart upload (like a form)
curl -X DELETE http://localhost:8080/upload/file.txt              # DELETE
curl -X PUT http://localhost:8080/                                # unsupported method → expect 405/501
```

## Headers

```bash
curl -H "Host: example.com" http://localhost:8080/          # different Host (server_name)
curl -H "Transfer-Encoding: chunked" -d "hello" http://localhost:8080/upload  # chunked body
curl -H "Connection: close" http://localhost:8080/          # no keep-alive
curl --http1.0 http://localhost:8080/                       # send as HTTP/1.0
```

## Errors and limits

```bash
curl -v http://localhost:8080/nope.html                     # 404 + your custom error page?
curl -d "$(head -c 2000000 /dev/zero | tr '\0' 'a')" http://localhost:8080/upload  # large body → 413
curl --path-as-is http://localhost:8080/../../etc/passwd    # path traversal, must not serve the file
curl -L http://localhost:8080/old                           # follow redirect (301/302)
```

## Keep-alive and timeouts

```bash
curl -v http://localhost:8080/ http://localhost:8080/       # two requests, look for "Re-using existing connection"
curl --max-time 5 http://localhost:8080/cgi-bin/slow.py     # give up if it hangs
```

## CGI

```bash
curl -v "http://localhost:8080/cgi-bin/hello.py/extra/path?name=value"  # PATH_INFO + QUERY_STRING
curl -X POST -d "a=1&b=2" http://localhost:8080/cgi-bin/hello.py         # body → CGI stdin
```

> **Tip:** `-w "%{http_code}\n"` is handy for test scripts. Loop over requests and compare status codes without reading the full output.

> **Note:** `-d` without `-X` makes curl switch to POST on its own and add `Content-Type: application/x-www-form-urlencoded`. Worth knowing if your parser checks Content-Type.


```bash
~> (printf 'GET /cgi-bin/sleep.py HTTP/1.1\r\nHost: localhost\r\n\r\n'; sleep 1;  printf 'GET / HTTP/1.1\r\nHost: localhost\r\n\r\n'; sleep 15) | nc 127.0.0.1 8080
```
check pipelining during cgi

```bash
(printf 'GET / HTTP/1.1\r\nHost: localhost\r\n\r\nGET / HTTP/1.1\r\nHost: localhost\r\n\r\n'; sleep 3) | nc 127.0.0.1 8080
```

--max-time5 while our parser didn't track amount of bytes

```bash
curl -s --max-time 5 -d "hello=world" http://127.0.0.1:8080/cgi-bin/echo_body.py
curl -s --max-time 5 -d "a=12&b=5&op=div" http://127.0.0.1:8080/cgi-bin/calc.py

```

idle send
to create /big.binm change root to www or what is in config: `dd if=/dev/zero of=<root>/big.bin bs=1m count=50`
```bash
(printf 'GET /big.bin HTTP/1.1\r\nHost: localhost\r\n\r\n'; sleep 120) | nc localhost 8080 | sleep 120
# or 
python3 www/cgi-bin/idle_send.py 
```

## strace in notes

## bash scripts
```text
The logger writes to the file only what passes through Logger::Log.
External stderr output does not end up there. 
To intercept it at startup:
```

`./webserv config/cgi 2>&1 | tee -a run.log`


____
> awk 'NR==1 || /[.]py/' - prints the header and lines containing `.py`;  
         the brackets stop `awk` from matching itself  
> [.]py -  script type  
> pid - process  
> ppid - parent process  
> stat - process state  
>        Z - zombie, finished but still in table  
>        R - running; if it lasts long, maybe an infinite loop,  
>        S - sleep and wait for event,  
>        T - stopped  
> etime - process lifetime  [[dd-]hh:]mm:ss  

`ps -ax -o pid,ppid,stat,etime,%cpu,command | awk 'NR==1 || /[w]ebserv|[.]py/ || $3 ~ /^Z/'`

```bash
while true; do
  clear
  date
  ps -ax -o pid,ppid,stat,etime,%cpu,command | awk 'NR==1 || /[w]ebserv|[.]py/ || $3 ~ /^Z/'
  sleep 1
done
```

## siege

Config file
~/.siege/siege.conf (created on first run) MacOS
~/.siege/siege.conf or /etc/siege/siegerc Ubuntu

Useful config keys in siege.conf:
• connection = keep-alive (default) reuses one TCP connection for several requests. Set connection = close if keep-alive is not implemented yet.
• protocol = HTTP/1.1 is the HTTP version siege sends.
• parser = false stops siege from also fetching images and CSS linked in the page.

| Flag | Meaning | Example |
| --- | --- | --- |
| `-c N` | Concurrent: N virtual clients at the same time (up to N open connections) | `-c 50` |
| `-t T` | Time: run for T, then stop and print the report. `S` = seconds, `M` = minutes, `H` = hours | `-t 30S`, `-t 1M` |
| `-r N` | Repetitions: each client sends N requests, then stop. Total = c × r. Use either `-t` or `-r`, not both | `-r 100` |
| `-b` | Benchmark: no pause between requests (maximum load) | `-b` |
| `-d N` | Delay: each client waits a random 0–N seconds between requests. Ignored with `-b` | `-d 1` |
| `-v` | Verbose: print every request and its status line | `-v` |
| `-f FILE` | Read URLs from a file, one per line | `-f urls.txt` |

Without `-t` and `-r`, siege runs until you press Ctrl+C. Ctrl+C at any time also prints the report.

Commands used for webserv:

```
# 1. Smoke test: does the basic path work?
siege -c 1 -r 10 http://localhost:8080/

# 2. Several clients at once
siege -c 10 -r 100 http://localhost:8080/

# 3. Eval-style stress test on a simple page
siege -b -c 50 -t 1M http://localhost:8080/

# 4. Large file: tests partial send() and the write offset
mkfile 10m www/big.bin            # macOS
fallocate -l 10M www/big.bin      # Ubuntu
siege -b -c 20 -t 1M http://localhost:8080/big.bin

# 5. Endurance: "siege indefinitely" - stop with Ctrl+C
siege -b -c 50 http://localhost:8080/

# Optional: POST
siege -c 5 -r 10 "http://localhost:8080/upload POST data=1"
```


## Monitoring the server

Run the server in one terminal, siege in a second, and the monitors below in a third. macOS has no `/proc` and no `watch` command, so it uses `lsof` and a shell loop.

| What | macOS | Ubuntu |
| --- | --- | --- |
| Open fd count, every second | `while true; do lsof -p $(pgrep webserv) \| wc -l; sleep 1; done` | `watch -n1 "ls /proc/$(pgrep webserv)/fd \| wc -l"` |
| TCP sockets only | `lsof -a -p $(pgrep webserv) -i TCP` | `lsof -a -p $(pgrep webserv) -i TCP` or `ss -tanp \| grep webserv` |
| Hanging connections | `lsof -a -p $(pgrep webserv) -i TCP \| grep CLOSE_WAIT` | same, or `ss -tanp state close-wait \| grep webserv` |
| Memory and CPU | `top -pid $(pgrep webserv)` | `top -p $(pgrep webserv)` |
| Zombie processes (CGI) | `while true; do ps -ef \| grep defunct \| grep -v grep; sleep 1; done` | `watch -n1 "ps -ef \| grep defunct \| grep -v grep"` |


`ulimit -n` controls how many file descriptors one process may open. It works the same on macOS and Ubuntu.

| Command | Effect |
| --- | --- |
| `ulimit -n` | Show the current limit |
| `ulimit -n 4096` | Set the limit for this terminal and the programs started from it |
| `ulimit -Hn` | Show the hard limit; the soft limit cannot go above it |

