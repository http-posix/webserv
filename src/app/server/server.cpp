#include "app/server/server.hpp"
#include "utils/logger/logger.hpp"
#include "utils/app_exception/app_exception.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cerrno>
#include <cstring>
#include <set>
#include <utility>
#include <cassert>

/* ========================================================================== */
/*                               Free Functions                               */
/* ========================================================================== */

// https://cplusplus.com/reference/set/set/insert/
std::vector<Server> CreateListeners(const Config& config){

	// Trust Boundary
	// don't need to check is config.empty() - Separation of Concerns - parser responsibility
	// Design by contract
	assert(!config.servers.empty() && "Parser allowed an empty server list, which violates the contract.");

	std::vector<Server>	listeners;
	std::set<std::pair<std::string, uint16_t>> unique_pairs;

	for (const ServerConfig& obj : config.servers){
		for (uint16_t port : obj.listen_ports){
			[[maybe_unused]] auto [ignored_iter, insert_result] = unique_pairs.insert({obj.host, port});
			if (!insert_result){
				LOG_ERROR("Duplicate configuration detected for server: " + obj.host + ":" + std::to_string(port));
				throw ServerException("Cannot bind multiple servers to the same host:port. Virtual hosts are not supported.");
			}
			listeners.push_back(Server(obj.host, port));
		}
	}
	return listeners;
}

/* ========================================================================== */
/*                             Anonymous Namespace                            */
/* ========================================================================== */

namespace {
	// Guard Class - provide RAII and avoid multiply freeaddrinfo calls
	struct AddrinfoGuard {
		addrinfo* ptr;

		explicit AddrinfoGuard(addrinfo* p):
		ptr(p)
		{}

		~AddrinfoGuard() {
			if (ptr)
				freeaddrinfo(ptr);
		}

		AddrinfoGuard(const AddrinfoGuard&) = delete;
		AddrinfoGuard& operator=(const AddrinfoGuard&) = delete;
	};

	// specify addrinfo struct for bind() call
	// getaddrinfo has individual set of errors – check manual page
	AddrinfoGuard SetupAddrinfo(const std::string& host, uint16_t port){
		addrinfo	hints{}; // to specify our needs

		hints.ai_family = AF_INET;// to use IPv4
		hints.ai_socktype = SOCK_STREAM;// to use TCP
		hints.ai_flags = AI_PASSIVE;// to create exactly server 

		addrinfo* res = nullptr;

		int ret_code = ::getaddrinfo(
			host.c_str(),
			std::to_string(port).c_str(),
			&hints,
			&res
		);

		if (ret_code != 0){
			LOG_ERROR("getaddrinfo() failed for " + host + ":" + std::to_string(port) + " | Error: " + std::string(::gai_strerror(ret_code)));
			throw ServerException("Failed to resolve server address configuration");
			}
		return AddrinfoGuard(res);
	}
}

/* ========================================================================== */
/*                               Public Methods                               */
/* ========================================================================== */
// sys_socket.h(0p), netinet_in.h(0p)
// => I need to keep config struct to give locations for Response
Server::Server(const std::string& host, uint16_t port){
	SetServerData(host, port);
	AddrinfoGuard addr_guard = SetupAddrinfo(host, port);
	SetupSocketOptions();
	BindSocket(addr_guard.ptr->ai_addr, addr_guard.ptr->ai_addrlen);
	ListenSocket();
}

/* ========================================================================== */
/*                             Accessors & Mutators                           */
/* ========================================================================== */

int	Server::fd() const noexcept{
	return socket_.fd();
}

uint16_t	Server::server_port() const noexcept{
	return port_;
}

const std::string&	Server::server_host() const noexcept{
	return host_;
}

/* ========================================================================== */
/*                              Private Methods                               */
/* ========================================================================== */
// keep server data for debug
void	Server::SetServerData(const std::string& host, uint16_t port){
	host_ = host;
	port_ = port;
}

void	Server::SetupSocketOptions(){
	int option_value = 1;
	int ret_code = ::setsockopt(
		socket_.fd(),
		SOL_SOCKET,// general level/layot of socket settings
		SO_REUSEADDR,// avoid EADDRINUSE on restart, socket may be in TIME_WAIT
		&option_value,
		sizeof(option_value)
	);
	if (ret_code == -1){
		LOG_ERROR("setsockopt() failed to set SO_REUSEADDR: " + std::string(strerror(errno)));
		throw ServerException("Failed to configure socket options");
	}
}

void	Server::BindSocket(const sockaddr* ai_addr, socklen_t ai_addrlen){
	int ret_code = ::bind(
		socket_.fd(),
		ai_addr,
		ai_addrlen
	);
	if (ret_code == -1){
		LOG_ERROR("bind() failed for " + host_ + ":" + std::to_string(port_) + ": " + std::string(strerror(errno)));
		throw ServerException("Failed to bind socket to address");
	}
}

void	Server::ListenSocket(){
	int ret_code = ::listen(socket_.fd(), SOMAXCONN);
	if (ret_code == -1){
		LOG_ERROR("listen() failed on socket fd " + std::to_string(socket_.fd()) + ": " + std::string(strerror(errno)));
		throw ServerException("Failed to put server into listening mode");
	}
}

// struct addrinfo {
// 	int              ai_flags;
// 	int              ai_family;
// 	int              ai_socktype;
// 	int              ai_protocol;
// 	socklen_t        ai_addrlen;
// 	struct sockaddr *ai_addr;
// 	char            *ai_canonname;
// 	struct addrinfo *ai_next;
// };

// int getaddrinfo(
// 	const char*			node,
// 	const char*			service,
// 	const addrinfo*		hints,
// 	addrinfo**			res
// );