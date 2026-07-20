#include "io/socket/socket.hpp"
#include <unistd.h>
#include "utils/app_exception/app_exception.hpp"
#include "utils/logger/logger.hpp"
#include <fcntl.h>
#include <cassert>
#include <cerrno>
#include <cstring>

// sockfd = socket(int socket_family, int socket_type, int protocol);
// global scope resolution for syscalls

// Used for Server socket
Socket::Socket(){
	sockfd_ = ::socket(default_domain, default_type, default_protocol); 
	if (sockfd_ == kInvalidFd){
		LOG_ERROR("Socket constructor");
		throw  ServerException("Socket fails");
	}
	if (SetNonBlockingMode() != 0){
		// LOG_ERROR() happened in SetNonBlockingMode()
		::close(sockfd_);
		throw  ServerException("Failed to set socket into non-blocking mode");
	}
}

Socket::~Socket(){
	if (sockfd_ != kInvalidFd)
		::close(sockfd_);
}

// Wraps an already-open, valid fd from accept()
// PRECONDITION: fd must be >= 0
// POSTCONDITION: returned Socket's socket_fd() may be kInvalidFd if
// internal setup (non-blocking mode) failed — caller must check before use
Socket Socket::adopt(int fd){
	assert((fd >= 0) && "Adopt method received an invalid fd, which violates the contract");
	return Socket(fd);
}

int Socket::fd() const {
	return sockfd_;
}

// Move
Socket::Socket(Socket&& other) noexcept:
	sockfd_(other.sockfd_)
{
	other.sockfd_ = kInvalidFd;
}

Socket& Socket::operator=(Socket&& other) noexcept{
	if (this != &other){
		if (sockfd_ != kInvalidFd)
			::close(sockfd_);
		sockfd_ = other.sockfd_;
		other.sockfd_ = kInvalidFd;
	}
	return *this;
}

// Private
// used after accept() for connections
Socket::Socket(int fd) {
	sockfd_ = fd;
	if (SetNonBlockingMode() != 0){
		::close(sockfd_);
		sockfd_ = kInvalidFd;
	}
}

int	Socket::SetNonBlockingMode() {
// read-modify-write pattern is a best practice in POSIX
// but flag F_GETFL isn't allowed in this project
// in this particular project this is ok, 'cause we set flags on just created socket wich hasn't any other flags
/* 
	int flags = ::fcntl(sockfd_, F_GETFL, 0);
	if (flags == -1)
	{
		LOG_ERROR("fcntl(F_GETFL) failed on socket fd " + std::to_string(sockfd_) + ": " + std::string(strerror(errno)));
		return 1;
	}
*/
	int ret_code = ::fcntl(sockfd_, F_SETFL, O_NONBLOCK);
	if (ret_code == -1){
		LOG_ERROR("fcntl(F_SETFL) failed on socket fd " + std::to_string(sockfd_) + ": " + std::string(strerror(errno)));
		return 1;
	}
	return 0;
}
