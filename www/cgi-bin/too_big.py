import sys

LIMIT = 10 * 1024 * 1024


def main():
	sys.stdout.write("Content-Type: text/plain\r\n\r\n")
	sys.stdout.write("x" * (LIMIT + 1))


if __name__ == "__main__":
	main()