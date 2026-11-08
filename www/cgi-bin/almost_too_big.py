# CGI test: output just below the server limit (kMaxCgiOutput = 10 MiB).
# Expected: 200 OK. Pair for too_big.py: the limit must not trigger too early.

import sys

LIMIT = 10 * 1024 * 1024
HEADERS = "Content-Type: text/plain\r\n\r\n"


def main():
	sys.stdout.write(HEADERS)
	# headers + body together must stay below the limit
	sys.stdout.write("x" * (LIMIT - len(HEADERS) - 1024))


if __name__ == "__main__":
	main()
