#include "io/socket/socket.hpp"
#include <unistd.h>
#include <netinet/in.h>
#include <string>

// c++ -std=c++17 -Wall -Wextra -Werror echo_server.cpp ../src/socket/socket.cpp -I../src -o echo_server

// check port availability: utility netcat: nc -zv localhost 8080
// lsof -i :<port>

// int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);


// struct sockaddr_in {
// 	sa_family_t			sin_family;		address family: AF_INET
// 	in_port_t			sin_port;		port in network byte order
// 	struct in_addr		sin_addr;		internet address
// };

// struct in_addr {
// 	uint32_t       s_addr;     // address in network byte order
// };

// htons = host to network short (16-bit) — port
// htonl = host to network long (32-bit) — IPv4 address
// ntohs, ntohl — other way around

// curl http://localhost:8080

int main(){
	Socket			server;
	sockaddr_in		addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);
	addr.sin_addr.s_addr = INADDR_ANY;

	bind(server.socket_fd(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	listen(server.socket_fd(), 1);

	int client_fd = accept(server.socket_fd(), nullptr, nullptr);

	Socket	client_socket = Socket::adopt(client_fd);

	std::string response = 
	"HTTP/1.1 200 OK\r\n"
	"Content-Type: text/plain\r\n"
	"Content-Length: 39\r\n"
	"\r\n"
	"Hello, world!\nWe are happy to see you!\n";

	write(client_socket.socket_fd(), response.data(), response.size());
	return 0;
}
