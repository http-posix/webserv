#pragma once

#include "config/mock_config.hpp"
#include "io/socket/socket.hpp"
#include <vector>
#include <string>
#include <cstdint>

/**
 * One Server = one listening socket bound to a single host:port.
 * Throws ServerException on any setup failure (setup/socket/bind/listen).
 * Move-only (owns a Socket).
 */

class Server{
	public:
		Server() = delete;
		Server(const std::string& host, uint16_t port);
		~Server() noexcept = default;

		// Copy
		// Deleted in public section: prevents implicit use and gives explicit compiler error
		Server(const Server& other) = delete;
		Server& operator=(const Server& other) = delete;

		// Move
		Server(Server&& other) noexcept = default;
		Server& operator=(Server&& other) noexcept = default;

		int fd() const noexcept;

	private:
		Socket			socket_;
		std::string		host_;
		uint16_t		port_;

		void	SetServerData(const std::string& host, uint16_t port);
		void	SetupSocketOptions();
		void	BindSocket(const sockaddr* ai_addr, socklen_t ai_addrlen);
		void	ListenSocket();
};

//namespace a bit overkill for project scale
	std::vector<Server> CreateListeners(const Config& config);
