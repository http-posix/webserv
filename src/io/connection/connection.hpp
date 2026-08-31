#pragma once

#include "io/connection/connection_state.hpp"
#include "io/instruction.hpp"
#include "http/request/request.hpp"
#include "http/parser/parser.hpp"
#include "io/socket/socket.hpp"

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