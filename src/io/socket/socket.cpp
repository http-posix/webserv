#include "io/socket/socket.hpp"
#include "utils/app_exception/app_exception.hpp"
#include "utils/logger/logger.hpp"

#include <unistd.h>
#include <cassert>

// sockfd = socket(int socket_family, int socket_type, int protocol);

// Used for Server socket
Socket::Socket(){
	sockfd_ = ::socket(default_domain, default_type, default_protocol); 
	if (sockfd_ == posix::kInvalidFd){
		LOG_ERROR("Socket constructor");
		throw  ServerException("Socket fails");
	}
	if (!posix::SetNonBlockingMode(sockfd_, "socket fd")){
		// LOG_ERROR() happened in SetNonBlockingMode()
		::close(sockfd_);
		throw  ServerException("Failed to set socket into non-blocking mode");
	}
}

Socket::~Socket(){
	if (sockfd_ != posix::kInvalidFd)
		::close(sockfd_);
}

// Wraps an already-open, valid fd from accept()
// PRECONDITION: fd must be >= 0
// POSTCONDITION: returned Socket's socket_fd() may be posix::kInvalidFd if
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
	other.sockfd_ = posix::kInvalidFd;
}

Socket& Socket::operator=(Socket&& other) noexcept{
	if (this != &other){
		if (sockfd_ != posix::kInvalidFd)
			::close(sockfd_);
		sockfd_ = other.sockfd_;
		other.sockfd_ = posix::kInvalidFd;
	}
	return *this;
}

// Private
// used after accept() for connections
Socket::Socket(int fd) {
	sockfd_ = fd;
	if (!posix::SetNonBlockingMode(sockfd_, "socket fd")){
		::close(sockfd_);
		sockfd_ = posix::kInvalidFd;
	}
}
