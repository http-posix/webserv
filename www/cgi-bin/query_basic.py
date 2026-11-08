import os
import sys
from urllib.parse import parse_qs


def main():
	query = os.environ.get("QUERY_STRING", "")
	params = parse_qs(query)

	sys.stdout.write("Content-Type: text/plain\r\n\r\n")
	sys.stdout.write(f"QUERY_STRING: '{query}'\n")
	for key, values in params.items():
		sys.stdout.write(f"{key} = {', '.join(values)}\n")


if __name__ == "__main__":
	main()