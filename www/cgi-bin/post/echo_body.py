"""
echo_body.py - body integrity check.

The server must pass exactly CONTENT_LENGTH bytes to stdin.
The script compares the declared length with the bytes actually read,
computes sha256 (compare with `sha256sum file` on the client)
and shows the beginning of the body.

Catches: truncated body, extra bytes, wrong CONTENT_LENGTH,
body that never reached the child at all.
"""
import hashlib
import os
import sys

PREVIEW_BYTES = 200


def respond(status: str, content_type: str, body: bytes) -> None:
    out = sys.stdout.buffer
    out.write(f"Status: {status}\r\n".encode())
    out.write(f"Content-Type: {content_type}\r\n".encode())
    out.write(f"Content-Length: {len(body)}\r\n".encode())
    out.write(b"\r\n")
    out.write(body)
    out.flush()


def read_body() -> tuple[int, bytes]:
    raw = os.environ.get("CONTENT_LENGTH", "")
    expected = int(raw) if raw.isdigit() else 0
    # Read exactly `expected` bytes instead of waiting for EOF,
    # because the server might not close stdin in time.
    data = sys.stdin.buffer.read(expected) if expected > 0 else b""
    return expected, data


def main() -> None:
    method = os.environ.get("REQUEST_METHOD", "")
    if method != "POST":
        respond("405 Method Not Allowed", "text/plain",
                f"Expected POST, got '{method}'\n".encode())
        return

    expected, data = read_body()
    match = "MATCH" if expected == len(data) else "MISMATCH"

    lines = [
        f"REQUEST_METHOD : {method}",
        f"CONTENT_TYPE   : {os.environ.get('CONTENT_TYPE', '(unset)')}",
        f"CONTENT_LENGTH : {os.environ.get('CONTENT_LENGTH', '(unset)')}",
        f"bytes received : {len(data)}  -> {match}",
        f"sha256         : {hashlib.sha256(data).hexdigest()}",
        f"preview        : {data[:PREVIEW_BYTES]!r}",
        "",
    ]
    respond("200 OK", "text/plain; charset=utf-8", "\n".join(lines).encode())


if __name__ == "__main__":
    main()