#include "socket/socket.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include "app_exception/app_exception.hpp"
#include "logger/logger.hpp"
#include <fcntl.h>
#include <cassert>

// sockfd = socket(int socket_family, int socket_type, int protocol);
// global scope resolution for syscalls

Socket::Socket(){
	sockfd_ = ::socket(default_domain, default_type, default_protocol); 
	if (sockfd_ == kInvalidFd){
		LOG_ERROR("Socket constructor");
		throw  ServerException("Socket fails");
	}
	SetNonBlockingMode();
}

Socket::~Socket(){
	if (sockfd_ != kInvalidFd)
		::close(sockfd_);
}

Socket Socket::adopt(int fd) {
	assert((fd >= 0) && "Adopt method received an invalid fd, which violates the contract");
	return Socket(fd);
}

int Socket::socket_fd() const noexcept{
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
Socket::Socket(int fd){
	sockfd_ = fd;
	SetNonBlockingMode();
}

void	Socket::SetNonBlockingMode(){
	int flags = ::fcntl(sockfd_, F_GETFL, 0);
	if (flags == -1)
	{
		::close(sockfd_);
		LOG_ERROR("fcntl(F_GETFL) failed on socket fd " + std::to_string(sockfd_) + ": " + std::string(strerror(errno)));
		throw ServerException("Failed to get socket flags");
	}
	int ret_code = ::fcntl(sockfd_, F_SETFL, flags | O_NONBLOCK);
	if (ret_code == -1){
		::close(sockfd_);
		LOG_ERROR("fcntl(F_SETFL) failed on socket fd " + std::to_string(sockfd_) + ": " + std::string(strerror(errno)));
		throw ServerException("Failed to set socket into non-blocking mode");
	}
}
