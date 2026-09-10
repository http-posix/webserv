#include "io/connection/connection.hpp"
#include "io/instruction.hpp"
#include "utils/logger/logger.hpp"

#include "../tests/mock_files/mock_response.hpp"

#include <sys/socket.h>

// to catch possible bug is we'll forget to move fd from map
// auto [it, inserted] = clients_.emplace(fd, Connection(std::move(connection)));
// if (!inserted){
// 	// TODO: LOG_ERROR — дублікат fd у clients_, це баг у логіці закриття з'єднань
// }

/* ========================================================================== */
/*                          Constructors & Destructors                        */
/* ========================================================================== */

Connection::Connection(Socket socket, std::string srv_id, const ServerConfig& server_config) : 
		socket_(std::move(socket)),
		srv_id_(srv_id),
		server_config_(&server_config),
		state_(StateReading{})
{ }

/* ========================================================================== */
/*                               Public Methods                               */
/* ========================================================================== */

// ssize_t recv(size_t size;
//                         int sockfd, void buf[size], size_t size,
//                         int flags);

// Variable result is defined by default ctor with 0 size
// That give us "no option return" so fd would be active and wait for next check
InstructionList	Connection::OnReadable(){
	InstructionList		instructions;
	constexpr size_t	kBufSize = 4096;
	char				buf[kBufSize];

	ssize_t recv_bytes = ::recv(socket_.fd(), buf, kBufSize - 1, 0);

	LOG_DEBUG("request fd=" + std::to_string(socket_.fd()) + " recv_bytes=" + std::to_string(recv_bytes), srv_id_);

	if (recv_bytes < 0){
		// LOG_ERROR();
		instructions.Add(Action::CloseConnection, socket_.fd());
		return instructions;
	}
	else if (recv_bytes == 0){
		instructions.Add(Action::CloseConnection, socket_.fd());
		return instructions;
	}
	// Happy path
	buf[recv_bytes] = '\0';
	// LOG_DEBUG("recv bytes=" + std::to_string(recv_bytes));

	HttpParserState	status = http_parser_.Feed(buf);

	switch (status){
		case HttpParserState::NeedMoreData:
			LOG_DEBUG("state - NeedMoreData fd=" + std::to_string(socket_.fd()), srv_id_);
			return instructions; // No instructions => fd goes through run loop again + keep StateReading
		case HttpParserState::Complete:
		{
			#ifdef DEBUG_MODE
			std::string	ports;
			for (uint16_t port : server_config_->listen_ports)
				ports += std::to_string(port) + " ";
			LOG_DEBUG("ServerStruct contents: hostname: " + server_config_->hostname
				+ " ports: " + ports,
				srv_id_);
			#endif
			HttpResponse response(http_parser_.GetRequest(), server_config_);
			state_ = StateWriting{
				response.Serialize(), 0
			};
			LOG_DEBUG("state Complete - " + std::to_string(socket_.fd()));
			instructions.Add(Action::WaitWritable, socket_.fd());
			return instructions;
		}
>>>>>>> a665b73 (fix(connection): updated Response serialization to new Response Object)
		case HttpParserState::InvalidRequest:
			LOG_DEBUG("state - InvalidRequest fd=" + std::to_string(socket_.fd()), srv_id_);
			instructions.Add(Action::CloseConnection, socket_.fd());
			return instructions;
	}
	return instructions;
}


// ssize_t send(size_t size;
                    //   int sockfd, const void buf[size], size_t size,
					//  int flags);

InstructionList	Connection::OnWritable(){
	InstructionList	instructions;
	StateWriting&	w = std::get<StateWriting>(state_);
	const char*		data = w.buffer.data() + w.offset;
	size_t			len = w.buffer.size() - w.offset;
	ssize_t			send_bytes = ::send(socket_.fd(), data, len, 0);
	LOG_DEBUG("On Writable: fd=" + std::to_string(socket_.fd()), srv_id_);
	LOG_DEBUG(std::string("Sending: ") + std::to_string( send_bytes), srv_id_);
	LOG_DEBUG(std::string("Data size: ") + std::to_string(w.buffer.size()), srv_id_);
	if (send_bytes < 0)
	{
		// Track an error
		// can't use errno, so just close conenction
		LOG_ERROR("send_bytes < 0", srv_id_);
		instructions.Add(Action::CloseConnection, socket_.fd());
		return instructions;
	}
	w.offset += send_bytes;
	if (w.offset == w.buffer.size()){
		instructions.Add(Action::CloseConnection, socket_.fd());
		LOG_DEBUG("w.offset == w.buffer.size", srv_id_);
		// when keep alive logic will be implemented
		// state_ = StateReading{};
		// instructions.Add(Action::WaitReadable, socket_.fd());
	}
	return instructions;
}

InstructionList	Connection::OnCgi(){
	InstructionList	instructions;

	return instructions;
}

/* ========================================================================== */
/*                             Accessors & Mutators                           */
/* ========================================================================== */

const std::string	Connection::srv_id() const noexcept {
	return srv_id_;
}

// const ServerConfig&	Connection::server_config() const noexcept{
// 	return *server_config_;
// }
