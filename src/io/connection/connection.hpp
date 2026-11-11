#pragma once

#include "io/connection/connection_state.hpp"
#include "io/connection/connection_info.hpp"
#include "io/instruction.hpp"
#include "http/request.hpp"
#include "http/parser/parser.hpp"
#include "io/socket/socket.hpp"
#include "config/parser/parser.hpp"
#include "http/http_status_code.hpp"

#include <sys/types.h>
#include <chrono>

/**
 * One client connection as a state machine: Reading -> (CGI) -> Writing.
 *
 * - Owns the client socket and the HTTP parser.
 * - Reacts to events (readable, writable, CGI pipe readable/writable, CGI deadline)
 *   and returns instructions; it does not change the poll set itself.
 * - CGI with a request body: the body is written to the script stdin in
 *   chunks; the write end is closed after the last byte, so the script
 *   sees EOF.
 * - CGI: collects the output (max kMaxCgiOutput), aborts with 502 on a
 *   read error or too large output, and with 504 after kCgiTimeout.
 * - Data from the client during CGI is read and discarded, so a client
 *   disconnect is still seen.
 */
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
		InstructionList	OnCgiReadable();
		InstructionList	OnCgiWritable();
		InstructionList	HandleCgiDeadline(Clock::time_point now);


		const std::string	srv_id() const noexcept;
		bool				InStateCgi() const noexcept;
		int					cgi_read_fd() const noexcept;
		int					cgi_write_fd() const noexcept;
		pid_t				cgi_pid() const noexcept;
		// const ServerConfig&	server_config() const noexcept;

	private:
		static constexpr size_t					kBufSize = 4096;
		static constexpr std::chrono::seconds	kCgiTimeout{20};
		static constexpr size_t					kMaxCgiOutput = 10 * 1024 * 1024; // 2^10 = 1024 

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
		InstructionList	DiscardWhileCgi(ssize_t recv_bytes);
		InstructionList	AbortCgi(const StateCGI& state_cgi, HttpStatusCode code);
		InstructionList CloseCgiWriteFd(StateCGI& state_cgi);

		//bool				keep_alive_ ;
	};
