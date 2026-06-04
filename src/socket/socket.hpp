#pragma once
#include <sys/socket.h>

// RAII wrapper for a POSIX file descriptor (socket).
// Owns exactly one fd — closes it in the destructor.
// Move-only: copy is deleted to prevent double-close.
// Use adopt() to wrap an existing fd (e.g. returned by accept()).

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
		// Deleted in public section: prevents implicit use and gives explicit compiler error
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
