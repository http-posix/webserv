"""
calc.py - HTML form calculator (application/x-www-form-urlencoded).

Body: a=12&b=5&op=div
The result depends ONLY on the body, so it is obvious whether the body arrived.

Tests: urlencoded body parsing, error codes via the Status header
(the server must turn "Status: 400 ..." into the HTTP status line).
"""
import html
import os
import sys
from urllib.parse import parse_qs

OPS = {
    "add": ("+", lambda a, b: a + b),
    "sub": ("-", lambda a, b: a - b),
    "mul": ("*", lambda a, b: a * b),
    "div": ("/", lambda a, b: a / b),
}


def respond(status: str, body_html: str) -> None:
    body = f"<!doctype html><html><body>{body_html}</body></html>\n".encode()
    out = sys.stdout.buffer
    out.write(f"Status: {status}\r\n".encode())
    out.write(b"Content-Type: text/html; charset=utf-8\r\n")
    out.write(f"Content-Length: {len(body)}\r\n\r\n".encode())
    out.write(body)
    out.flush()


def read_body() -> bytes:
    raw = os.environ.get("CONTENT_LENGTH", "")
    length = int(raw) if raw.isdigit() else 0
    return sys.stdin.buffer.read(length) if length > 0 else b""


def main() -> None:
    if os.environ.get("REQUEST_METHOD") != "POST":
        respond("405 Method Not Allowed", "<h1>405</h1><p>Use POST</p>")
        return

    ctype = os.environ.get("CONTENT_TYPE", "")
    if not ctype.startswith("application/x-www-form-urlencoded"):
        respond("415 Unsupported Media Type",
                f"<h1>415</h1><p>Got: {html.escape(ctype or '(unset)')}</p>")
        return

    form = parse_qs(read_body().decode("utf-8", errors="replace"))
    try:
        a = float(form["a"][0])
        b = float(form["b"][0])
        symbol, fn = OPS[form["op"][0]]
        result = fn(a, b)
    except (KeyError, IndexError, ValueError):
        respond("400 Bad Request",
                "<h1>400</h1><p>Need a, b (numbers) and op in add|sub|mul|div</p>")
        return
    except ZeroDivisionError:
        respond("400 Bad Request", "<h1>400</h1><p>Division by zero</p>")
        return

    respond("200 OK", f"<h1>{a:g} {symbol} {b:g} = {result:g}</h1>")


if __name__ == "__main__":
    main()