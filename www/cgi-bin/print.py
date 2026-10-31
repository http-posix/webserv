# Minimal CGI script for GET testing.
# Shows what the server passed via environment variables (RFC 3875).

import os
import sys
from urllib.parse import parse_qs

method = os.environ.get("REQUEST_METHOD", "")
query = os.environ.get("QUERY_STRING", "")
params = parse_qs(query)

body = []
body.append("<html><body>")
body.append("<h1>Hello from Python CGI</h1>")
body.append(f"<p>REQUEST_METHOD: {method}</p>")
body.append(f"<p>QUERY_STRING: {query}</p>")

if params:
    body.append("<ul>")
    for key, values in params.items():
        body.append(f"<li>{key} = {', '.join(values)}</li>")
    body.append("</ul>")

body.append("<h2>CGI env</h2><ul>")
for key, value in sorted(os.environ.items()):
    body.append(f"<li>{key} = {value}</li>")
body.append("</ul></body></html>")

html = "\n".join(body)

# CGI output: headers, empty line, body
sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write(f"Content-Length: {len(html.encode())}\r\n")
sys.stdout.write("\r\n")
sys.stdout.write(html)