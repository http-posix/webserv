// class Connection{
// 	HttpParser obj;
	
// 	HttpResponse obj2();
// 	httpstate state = obj.Feed();
// 	if state == COMPLETE
// 		obj2.FillResponse(obj);
// };

#pragma once

#include "connection_state.hpp"
#include "connection_result.hpp"
#include "http_request/http_request.hpp"
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
		ConnectionResult	OnReadable();
		ConnectionResult	OnWritable();
		ConnectionResult	OnCgi();

	private:
		ConnectionState		state_;
		HttpParser			http_parser_;
		// HttpResponse		http_response_; - note for future, when it would be ready
};