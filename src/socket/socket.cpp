#include "socket/socket.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include "app_exception/app_exception.hpp"
#include "logger/logger.hpp"

// sockfd = socket(int socket_family, int socket_type, int protocol);
// global scope resolution for syscalls

Socket::Socket(){

	sockfd_ = ::socket(default_domain, default_type, default_protocol); 
	if (sockfd_ == kInvalidFd){
		LOG_ERROR("Socket constructor");
		throw  ServerException("Socket fails");
	}
}

Socket::~Socket(){
	if (sockfd_ != kInvalidFd)
		::close(sockfd_);
}

Socket Socket::adopt(int fd) {
    return Socket(fd);
}

int Socket::socket_fd() const{
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
}
