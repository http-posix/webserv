import sys
import time

def main():
	time.sleep(10)
	sys.stdout.write("Content-Type: text/plain\r\n")
	sys.stdout.write("\r\n")
	sys.stdout.write("Woke up after 10 seconds\n")

if __name__ == "__main__":
	main()