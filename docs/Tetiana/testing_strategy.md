TESTING STRATEGY FOR THE WHOLE PROJECT
======================================

Summary
-------
Add an integration test harness next to the existing unit tests.

The doctest suite tests classes in isolation. A webserver is evaluated
by its behaviour from the outside (curl, browser, siege). Most real bugs
come from parts working together: the event loop, partial reads and
writes, Connection state changes, CGI and config. Unit tests cannot see
these bugs.

Example: the std::bad_variant_access crash in the CGI flow was not
caught by cgi_test, because cgi_test tests the Cgi class alone.


Two test levels
---------------
Level         What it checks                            Tool                Status
-----------   ---------------------------------------   -----------------   -------------
Unit          Parser, router, config, Cgi on their own  doctest, make test  already done
Integration   The real ./webserv binary over sockets    new harness,        missing
                                                        make itest


Harness design
--------------
Language
  - A Python script that uses only the standard library
    (subprocess, socket, http.client).
  - Bash with curl is fine for about 5 checks. It becomes hard when
    you need things curl cannot do:
      * a request split across several send() calls
      * broken requests
      * a client that disconnects in the middle of a request
      * timeouts
    A raw socket can do all of these.
  - Python 3 is already on the CI runner, so CI needs no extra setup.

Layout
  tests/integration/
      run.py      starts the server, runs the cases, prints a summary,
                  exits 1 on failure
      configs/    one small config per feature area
      www/        fixture files: static pages, CGI scripts, upload dir

What each test run does
  1. Start the server with one config.
  2. Wait until the port accepts connections (no fixed sleep).
  3. Send the requests.
  4. Check the status code, important headers and the body.
  5. Check that the server is still alive after each case.
     This check would have caught the bad_variant_access crash.
  6. Stop the server.

Cases to add over time
  - static GET, index, autoindex
  - 404, 403, 405
  - redirects
  - body size limit
  - POST upload and DELETE
  - CGI: 200, 502, timeout
  - several listen ports and server blocks
  - partial or invalid requests
  - client disconnects in the middle of a request

CI
  - Add "make itest" as one more step in .github/workflows/ci.yml,
    after "make test".
  - CI runs only on push to main and on pull requests to main.
    WIP branches are not tested by CI.


Keep out of CI (run by hand)
----------------------------
- Stress tests (siege and similar). On shared runners the results are
  slow and change from run to run.
- Leak checks. On Linux, the integration suite can later run under
  valgrind in CI. On macOS, use "leaks --atExit" for manual checks.


When to start
-------------
- Start now with a small version: one config and 3-4 cases
  (a static page, a 404, one CGI script, the "server still alive"
  check).
- Add one or two cases each time a feature is merged.
- If the harness is written at the end of the project, it becomes a big
  task. If it grows with each feature, it stays small.
