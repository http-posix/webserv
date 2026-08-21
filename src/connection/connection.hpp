#pragma once

#include "connection_state.hpp"
#include "connection/instruction.hpp"
#include "http_request/http_request.hpp"
#include "http_parser/http_parser.hpp"
#include "socket/socket.hpp"

class Connection{
	public:
		Connection() = delete;
		Connection(Socket socket);
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

	private:
		Socket				socket_;
		ConnectionState		state_; // Should I be consistent and set state_ in ctor or better here, `cause this is always initial state?
		HttpParser			http_parser_; // Not in StateReadable to keep buffer with 2 dif requests
		// HttpRequest		http_request_;
		// HttpResponse		http_response_; - note for future, when it would be ready
		// Router			router_;
		// bool				keep_alive;
	};