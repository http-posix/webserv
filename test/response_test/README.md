# Response test harness

This folder contains a minimal C++ program to test the GET response generation for a static HTML file.

Files added:
- `main.cpp` — small program that reads `mock.conf` to get `root`, simulates a `GET /` request, and prints the serialized HTTP response to stdout.
- `mock.conf` — single-token file containing the filesystem root for test content.
- `mock_site/index.html` — sample HTML file returned by the handler.

Build and run:

```bash
cd test/response_test
g++ -std=c++17 main.cpp -o response_test
./response_test mock.conf > out.raw
# view headers and start of body
head -n 20 out.raw
```

`out.raw` will contain the full HTTP response (status line, headers, blank line, body). This lets you validate formatting and `Content-Length`.
