def main():
	body = "Hello! This is a script written in Python!\n"

	print("Content-Type: text/plain")
	print(f"Content-Length: {len(body.encode())}")
	# headers end:
	print()
	print(body, end="")

if __name__ == "__main__":
	main()