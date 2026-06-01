#pragma once
#include <sys/socket.h>


// Delete in public so in case developer will try to use it he'll get explicit behaviour
// AF_INET : IPv4 protocol
// SOCK_STREAM: TCP socket

class Socket{
	public:
		Socket();
		~Socket();

		// Static factory method
		// Copy elision. RVO — return value optimization
		static Socket adopt(int fd);

		//Copy
		Socket(const Socket& other) = delete;
		Socket& operator=(const Socket& other) = delete;
		
		// Move
		Socket(Socket&& other) noexcept;
		Socket& operator=(Socket&& other) noexcept;

		int socket_fd() const;

	private:
		int sockfd_;
		explicit Socket(int fd);
		constexpr static int kInvalidFd = -1;
		constexpr static int default_domain = AF_INET;
		constexpr static int default_type = SOCK_STREAM;
		constexpr static int default_protocol = 0;
};
