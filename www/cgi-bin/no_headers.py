# CGI test: output without a header block.
# Expected: 502 Bad Gateway (no empty line that ends the headers).

import sys


def main():
	sys.stdout.write("This is a body without any headers.\n")


if __name__ == "__main__":
	main()
