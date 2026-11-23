# Minimal CGI script for GET testing.
# Prints plain text so the output is easy to check with curl.

# Content-Type is required when there is a body.
# printf, not echo: exact bytes, \r\n is not altered.
printf "Content-Type: text/plain\r\n"
# Empty line = end of headers. Server splits CGI output here.
printf "\r\n"

echo "Hello from Bash CGI"
env | sort
echo
echo "Date: $(date)"
