"""
word_stats.py - JSON API: text analysis.

Body:     {"text": "...", "top": 5}
Response: JSON with statistics (chars, words, lines, most common words).

Tests: large bodies (send a file of hundreds of KB), UTF-8 (non-ASCII text),
415 on wrong Content-Type, 400 on malformed JSON.
"""
import json
import os
import re
import sys
from collections import Counter

DEFAULT_TOP = 5
WORD_RE = re.compile(r"\w+", re.UNICODE)


def respond(status: str, payload: dict) -> None:
    body = (json.dumps(payload, ensure_ascii=False, indent=2) + "\n").encode()
    out = sys.stdout.buffer
    out.write(f"Status: {status}\r\n".encode())
    out.write(b"Content-Type: application/json; charset=utf-8\r\n")
    out.write(f"Content-Length: {len(body)}\r\n\r\n".encode())
    out.write(body)
    out.flush()


def read_body() -> bytes:
    raw = os.environ.get("CONTENT_LENGTH", "")
    length = int(raw) if raw.isdigit() else 0
    return sys.stdin.buffer.read(length) if length > 0 else b""


def main() -> None:
    if os.environ.get("REQUEST_METHOD") != "POST":
        respond("405 Method Not Allowed", {"error": "use POST"})
        return

    if not os.environ.get("CONTENT_TYPE", "").startswith("application/json"):
        respond("415 Unsupported Media Type", {"error": "expected application/json"})
        return

    try:
        data = json.loads(read_body().decode("utf-8"))
        text = data["text"]
        top = int(data.get("top", DEFAULT_TOP))
        if not isinstance(text, str):
            raise TypeError
    except (UnicodeDecodeError, json.JSONDecodeError, KeyError, TypeError, ValueError):
        respond("400 Bad Request", {"error": 'body must be JSON like {"text": "..."}'})
        return

    words = [w.lower() for w in WORD_RE.findall(text)]
    respond("200 OK", {
        "chars": len(text),
        "bytes_utf8": len(text.encode("utf-8")),
        "words": len(words),
        "unique_words": len(set(words)),
        "lines": text.count("\n") + 1 if text else 0,
        "top": Counter(words).most_common(top),
    })


if __name__ == "__main__":
    main()