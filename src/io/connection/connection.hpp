#pragma once

#include "io/connection/connection_state.hpp"
#include "io/connection/connection_info.hpp"
#include "io/instruction.hpp"
#include "http/request.hpp"
#include "http/parser/parser.hpp"
#include "io/socket/socket.hpp"
#include "config/parser/parser.hpp"
#include "http/http_status_code.hpp"

class Connection{
	public:
		Connection() = delete;
		Connection(Socket socket, const ConnInfo& conn_info, const std::string& srv_id, const ServerConfig& server_config);
		~Connection() = default;

		// copy
		Connection(const Connection& other) = delete;
		Connection& operator=(const Connection& other) = delete;

		// move
		Connection(Connection&& other) noexcept = default;
		Connection& operator=(Connection&& other) noexcept = default;

		// methods
		InstructionList	OnReadable();
		InstructionList	OnWritable();
		InstructionList	OnCgi();


		const std::string	srv_id() const noexcept;
		// const ServerConfig&	server_config() const noexcept;

	private:
		static constexpr size_t	kBufSize = 4096;

		Socket				socket_;
		ConnInfo			conn_info_;
		std::string			srv_id_;
		const ServerConfig*	server_config_;

		ConnectionState		state_ = StateReading{};
		HttpParser			http_parser_; // Not in StateReadable to keep buffer with 2 dif requests

		// methods
		InstructionList ProcessReceivedBytes(const char* buf);
		InstructionList	HandleCompleteRequest();
		InstructionList	ProcessCgiOutput(StateCGI& cgi);
		InstructionList	FinishCgi(StateCGI& cgi, const HttpStatusCode& code);

		// bool				keep_alive;
	};
