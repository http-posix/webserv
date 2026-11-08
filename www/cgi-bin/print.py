# Minimal CGI script for GET testing.
# Shows what the server passed via environment variables (RFC 3875).

import html
import os

CGI_VARS = {
	"GATEWAY_INTERFACE", "QUERY_STRING", "REMOTE_ADDR", "REMOTE_HOST",
	"REQUEST_METHOD", "SCRIPT_NAME", "SERVER_NAME", "SERVER_PORT",
	"SERVER_PROTOCOL", "SERVER_SOFTWARE",
	"CONTENT_LENGTH", "CONTENT_TYPE", "PATH_INFO",
}


def is_ours(key):
	return key in CGI_VARS or key.startswith("HTTP_")


def print_header():
	print("Content-Type: text/html\r\n\r\n", end="")

def print_our_vars():
	print("<h2>CGI env (set by webserv)</h2><ul>")
	for key in sorted(k for k in os.environ if is_ours(k)):
		print(f"<li>{html.escape(key)} = {html.escape(os.environ[key])}</li>")
	print("</ul>")


def print_other_names():
	others = sorted(k for k in os.environ if not is_ours(k))
	names = ", ".join(html.escape(k) for k in others) or "none"
	print(f"<p><small>Not from webserv: {names}</small></p>")

def main():
	print_header()
	print_our_vars()
	print_other_names()

if __name__ == "__main__":
	main()