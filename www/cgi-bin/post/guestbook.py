"""
guestbook.py - guestbook with state persisted between requests.

POST name=Tetiana&message=Hello  -> appends an entry to a file
GET                              -> shows all entries

The file is written to the RELATIVE path ./data/guestbook.txt,
so this also checks that the server did chdir() into the script's directory:
the file must appear next to the script (www/cgi-bin/data/),
not where webserv was launched from.

Tests: state between requests, chdir, 413 on a too-long message,
HTML escaping (try message=<script>alert(1)</script>).
"""
import html
import os
import sys
from datetime import datetime
from urllib.parse import parse_qs

DATA_DIR = "data"
DATA_FILE = os.path.join(DATA_DIR, "guestbook.txt")
MAX_MESSAGE = 500
SEP = "\t"


def respond(status: str, body_html: str) -> None:
    body = (
        "<!doctype html><html><head><meta charset='utf-8'>"
        "<title>Guestbook</title></head><body>"
        f"{body_html}</body></html>\n"
    ).encode()
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


def clean(value: str) -> str:
    # Tabs and newlines would break the file format (one entry = one line)
    return value.replace(SEP, " ").replace("\r", " ").replace("\n", " ").strip()


def add_entry() -> str | None:
    """Returns an error status string, or None on success."""
    form = parse_qs(read_body().decode("utf-8", errors="replace"))
    name = clean(form.get("name", ["anonymous"])[0]) or "anonymous"
    message = clean(form.get("message", [""])[0])
    if not message:
        return "400 Bad Request"
    if len(message) > MAX_MESSAGE:
        return "413 Content Too Large"

    os.makedirs(DATA_DIR, exist_ok=True)
    stamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    with open(DATA_FILE, "a", encoding="utf-8") as f:
        f.write(f"{stamp}{SEP}{name}{SEP}{message}\n")
    return None


def render_entries() -> str:
    if not os.path.exists(DATA_FILE):
        return "<p><i>No entries yet.</i></p>"
    items = []
    with open(DATA_FILE, encoding="utf-8") as f:
        for line in f:
            parts = line.rstrip("\n").split(SEP, 2)
            if len(parts) == 3:
                stamp, name, message = (html.escape(p) for p in parts)
                items.append(f"<li><b>{name}</b> <small>{stamp}</small><br>{message}</li>")
    return "<ul>" + "".join(reversed(items)) + "</ul>"


def main() -> None:
    method = os.environ.get("REQUEST_METHOD", "")
    status = "200 OK"

    if method == "POST":
        error = add_entry()
        if error:
            respond(error, f"<h1>{error}</h1><p>message: 1..{MAX_MESSAGE} chars</p>")
            return
        status = "201 Created"
    elif method != "GET":
        respond("405 Method Not Allowed", "<h1>405</h1>")
        return

    respond(status,
            f"<h1>Guestbook</h1><p>cwd: {html.escape(os.getcwd())}</p>"
            + render_entries())


if __name__ == "__main__":
    main()