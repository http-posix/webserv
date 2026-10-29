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