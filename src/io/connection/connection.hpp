#pragma once

#include "io/connection/connection_state.hpp"
#include "io/instruction.hpp"
#include "http/request/request.hpp"
#include "http/parser/parser.hpp"
#include "io/socket/socket.hpp"
#include "config/parser/parser.hpp"

class Connection{
	public:
		Connection() = delete;
		Connection(Socket socket, const std::string& addr, const std::string& srv_id, const ServerConfig& server_config);
		~Connection() = default;

		// copy
		Connection(const Connection& other) = delete;
		Connection& operator=(const Connection& other) = delete;

		//move
		Connection(Connection&& other) noexcept = default;
		Connection& operator=(Connection&& other) noexcept = default;

		// methods
		InstructionList	OnReadable();
		InstructionList	OnWritable();
		InstructionList	OnCgi();
		InstructionList ProcessReceivedBytes(const char* buf);
		InstructionList	HandleCompleteRequest();

		const std::string	srv_id() const noexcept;
		// const ServerConfig&	server_config() const noexcept;

	private:
		Socket				socket_;
		[[maybe_unused]] std::string			addr_; //remove [[maybe_unused]] once CGI wired
		std::string			srv_id_;

		// Used by response building (WIP); remove [[maybe_unused]] once wired
		[[maybe_unused]] const ServerConfig*	server_config_;

		ConnectionState		state_ = StateReading{};

		HttpParser			http_parser_; // Not in StateReadable to keep buffer with 2 dif requests
		
		// HttpRequest		http_request_;
		// HttpResponse		http_response_; - note for future, when it would be ready
		// Router			router_;
		// bool				keep_alive;
	};
