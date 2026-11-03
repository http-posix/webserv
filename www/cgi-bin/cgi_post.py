import sys

def main():
	body = sys.stdin.read()

	print("Content-Type: text/plain")
	print()

	if body:
		print(f"Received: {body}")
	else:
		print("No data received. This script expects a POST request with a body.")

if __name__ == "__main__":
	main()