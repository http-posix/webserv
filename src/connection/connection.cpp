#include "connection/connection.hpp"
#include "connection/instruction.hpp"
#include "logger/logger.hpp"

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

Connection::Connection(Socket socket) : 
		socket_(std::move(socket)),
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
			LOG_DEBUG("state NeedMoreData - " + std::to_string(socket_.fd()));
			return instructions; // No instructions => fd goes through run loop again + keep StateReading
		case HttpParserState::Complete:
			// Send parsed data to the Router
			// Router return RouterInstruction/State/Result/Code/etc
			// according to the return need to decide is it CGI => 
			// state_ = StateCgi{?, ?};
			// instructions.Add(Action::WatchCgi, socket_.fd());
			// else =>
			state_ = StateWriting{BuildMockResponse(), 0};
			LOG_DEBUG("state Complete - " + std::to_string(socket_.fd()));
			instructions.Add(Action::WaitWritable, socket_.fd());
			return instructions;
		case HttpParserState::InvalidRequest:
			LOG_DEBUG("state InvalidRequest - " + std::to_string(socket_.fd()));
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
	if (send_bytes < 0)
	{
		// Track an error
		// can't use errno, so just close conenction
		// LOG_ERROR();
		instructions.Add(Action::CloseConnection, socket_.fd());
		return instructions;
	}
	w.offset += send_bytes;
	if (w.offset == w.buffer.size()){
		instructions.Add(Action::CloseConnection, socket_.fd());
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
/*                              Private Methods                               */
/* ========================================================================== */
