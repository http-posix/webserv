import socket
import time

def main():
	s = socket.create_connection(("localhost", 8080))
	s.sendall(b"GET /big.bin HTTP/1.1\r\nHost: localhost\r\n\r\n")
	time.sleep(120)

if __name__ == "__main__":
	main()