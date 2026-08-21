#include "connection/connection.hpp"

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

// ssize_t recv(size_t size;
//                         int sockfd, void buf[size], size_t size,
//                         int flags);

/* ========================================================================== */
/*                               Public Methods                               */
/* ========================================================================== */

// Variable result is defined by default ctor with 0 size
// That give us "no option return" so fd would be active and wait for next check
ConnectionResult	Connection::OnReadable(){
	ConnectionResult	result;
	constexpr size_t	kBufSize = 4096;
	char				buf[kBufSize];

	ssize_t recv_bytes = ::recv(socket_.fd(), buf, kBufSize - 1, 0);

	if (recv_bytes < 0){
		// LOG_ERROR();
		result.instructions.push_back({Action::CloseConnection, socket_.fd()});
		return result;
	}
	else if (recv_bytes == 0){
		result.instructions.push_back({Action::CloseConnection, socket_.fd()});
		return result;
	}
	// Happy path
	buf[recv_bytes] = '\0';
	HttpParserState	status = http_parser_.Feed(buf);

	if (status == HttpParserState::NeedMoreData){
		return result;
	}
	else if (status == HttpParserState::Complete){
		state_ = StateWriting{};
		result.instructions.push_back({Action::WatchWritable, socket_.fd()});
		return result;
	}
	else if (status == HttpParserState::InvalidRequest){
		result.instructions.push_back({Action::CloseConnection, socket_.fd()});
		return result;
	}
	return result;
}

ConnectionResult OnWritable(){

}

ConnectionResult OnCgi(){

}

/* ========================================================================== */
/*                              Private Methods                               */
/* ========================================================================== */
