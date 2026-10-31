#include "io/connection/connection.hpp"
#include "io/instruction.hpp"
#include "utils/logger/logger.hpp"
#include "http/response/response.hpp"
#include "http/router/router.hpp"
#include "config/parser/parser.hpp"
#include "cgi/cgi.hpp"

#include <sys/socket.h>
#include <unistd.h>
#include <array>
#include <cassert>

namespace {
	// TODO: how this could be replaced this with reasponse http?
	std::string	BuildCgiError(const HttpStatusCode code){
		std::string	body = std::to_string(code) + "\n";

		return "HTTP/1.1 " + std::to_string(code) + " \r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: " + std::to_string(body.size()) + "\r\n"
			"Connection: close\r\n"
			"\r\n" + body;
	}

	// Later need to check which code returned in the header
	std::string	BuildCgiResponse(const std::string& cgi_output){
		std::string	mock_header = "HTTP/1.1 200 OK\r\n";
		return mock_header + cgi_output;
	}

	bool	HasHeaderEnd(const std::string& output){
		return output.find("\r\n\r\n") != std::string::npos
			|| output.find("\n\n") != std::string::npos;
	}
}

// to catch possible bug is we'll forget to move fd from map
// auto [it, inserted] = clients_.emplace(fd, Connection(std::move(connection)));
// if (!inserted){
// 	// TODO: LOG_ERROR — duplicate fd in clients_
// }

/* ========================================================================== */
/*                          Constructors & Destructors                        */
/* ========================================================================== */

Connection::Connection(Socket socket, const ConnInfo& conn_info, const std::string& srv_id, const ServerConfig& server_config) : 
		socket_(std::move(socket)),
		conn_info_(conn_info),
		srv_id_(srv_id),
		server_config_(&server_config)
{ }

/* ========================================================================== */
/*                               Public Methods                               */
/* ========================================================================== */

// Variable result is defined by default ctor with 0 size
// That give us "no option return" so fd would be active and wait for next check
InstructionList	Connection::OnReadable(){
	InstructionList		instructions;
	char				buf[kBufSize];

	ssize_t recv_bytes = ::recv(socket_.fd(), buf, kBufSize - 1, 0);

	if (recv_bytes < 0){
		LOG_WARN("reading: recv failed, closing fd=" + std::to_string(socket_.fd()), srv_id_);
		instructions.Add(Action::CloseConnection, socket_.fd());
		return instructions;
	}
	if (recv_bytes == 0){
		LOG_DEBUG("reading: client closed connection, closing fd=" + std::to_string(socket_.fd()), srv_id_);
		instructions.Add(Action::CloseConnection, socket_.fd());
		return instructions;
	}

	LOG_DEBUG("reading: recv fd=" + std::to_string(socket_.fd())
		+ " recv_bytes=" + std::to_string(recv_bytes), srv_id_);
	buf[recv_bytes] = '\0';
	return ProcessReceivedBytes(buf);
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
		LOG_ERROR("writing: send_bytes < 0, fd=" + std::to_string(socket_.fd()), srv_id_);
		instructions.Add(Action::CloseConnection, socket_.fd());
		return instructions;
	}

	w.offset += send_bytes;
	LOG_DEBUG("writing: send fd=" + std::to_string(socket_.fd())
			+ " send_bytes=" + std::to_string(send_bytes)
			+ " progress=" + std::to_string(w.offset)
			+ "/" + std::to_string(w.buffer.size()),
			srv_id_);

	if (w.offset == w.buffer.size()){
		LOG_DEBUG("writing: response fully sent, closing fd=" + std::to_string(socket_.fd()), srv_id_);
		instructions.Add(Action::CloseConnection, socket_.fd());
		// when keep alive logic will be implemented
		// state_ = StateReading{};
		// instructions.Add(Action::WaitReadable, socket_.fd());
	}
	return instructions;
}

InstructionList	Connection::OnCgi(){
	InstructionList					instructions;

	StateCGI&						state_cgi = std::get<StateCGI>(state_);
	std::array <char, kBufSize>		buf;

	ssize_t bytes = ::read(state_cgi.pipe_read_fd, buf.data(), buf.size());

	if (bytes < 0){
		LOG_ERROR("cgi: read failed fd=" + std::to_string(socket_.fd())
				+ " pid=" + std::to_string(state_cgi.child_pid),
				srv_id_);
		// TODO stop child
		instructions.Add(Action::StopWatching, state_cgi.pipe_read_fd);
		instructions.Add(Action::WaitWritable, socket_.fd());

		::close(state_cgi.pipe_read_fd);

		// TODO check is code proper for this type of error
		state_ = StateWriting{BuildCgiError(HttpStatusCode::BadGateway), 0};
		return instructions;
	}
	if (bytes > 0){
		state_cgi.output.append(buf.data(), bytes); // bytes automatically will be casted to the size_t
		return instructions; // like in OnReadable() no instructions, because we are waiting for more data
	}
	return ProcessCgiOutput(state_cgi);
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

/* ========================================================================== */
/*                              Private Methods                               */
/* ========================================================================== */

InstructionList Connection::ProcessReceivedBytes(const char* buf){
	InstructionList		instructions;
	HttpParserState	status = http_parser_.Feed(buf);

	switch (status){
		case HttpParserState::NeedMoreData:
			LOG_DEBUG("reading: request incomplete, waiting for more data fd="
				+ std::to_string(socket_.fd()), srv_id_);
			break; // No instructions => fd goes through run loop again + keep StateReading
		case HttpParserState::Complete:
			instructions = HandleCompleteRequest();
			break;
		case HttpParserState::InvalidRequest:
			LOG_DEBUG("reading: invalid request, closing fd=" + std::to_string(socket_.fd()), srv_id_);
			instructions.Add(Action::CloseConnection, socket_.fd());
			break;
	}
	return instructions;
}

InstructionList	Connection::HandleCompleteRequest(){
	InstructionList			instructions;
	const HttpRequest&		request = http_parser_.GetRequest();
	HttpRouter				router(request, server_config_);

	auto routed_request = router.GetRoutedRequest();
	if (routed_request != std::nullopt){
			HttpResponse			response(server_config_);

			response.BuildResponse(*routed_request);
			instructions.Add(Action::WaitWritable, socket_.fd());
			state_ = StateWriting{response.Serialize(), 0};
			LOG_DEBUG("reading: request parsed -> writing fd=" + std::to_string(socket_.fd()), srv_id_);
	}
	else{
		auto cgi_request = router.GetCgiRequest();
		// invariant from router
		assert(cgi_request != std::nullopt);
	
		Cgi	cgi(conn_info_, request, *cgi_request, srv_id_);
		HttpStatusCode	status_code = cgi.SpawnChild();
	
		if (status_code != HttpStatusCode::OK){
			// TODO check flow for dynamic resources
			instructions.Add(Action::WaitWritable, socket_.fd());
			state_ = StateWriting{BuildCgiError(status_code), 0};
			LOG_WARN("reading: cgi spawn failed fd=" + std::to_string(socket_.fd()), srv_id_);
		}
		else{
			instructions.Add(Action::WatchCgi, cgi.pipe_read_fd());
			state_ = StateCGI{cgi.pipe_read_fd(), cgi.child_pid()};
			LOG_DEBUG("reading: cgi request parsed -> cgi fd=" + std::to_string(socket_.fd())
					+ " pid=" + std::to_string(cgi.child_pid()),
					srv_id_);
		}
	}
	return instructions;
}

InstructionList	Connection::ProcessCgiOutput(StateCGI& state_cgi){
	InstructionList		instructions;

	const pid_t			pid = state_cgi.child_pid;

	instructions.Add(Action::StopWatching, state_cgi.pipe_read_fd);
	instructions.Add(Action::WaitWritable, socket_.fd());

	::close(state_cgi.pipe_read_fd);
	// TODO: reap child

	if (!HasHeaderEnd(state_cgi.output)){
		LOG_WARN("cgi: no headers in output pid=" + std::to_string(pid)
					+ " fd=" + std::to_string(socket_.fd()),
					srv_id_);
		state_ = StateWriting{BuildCgiError(HttpStatusCode::BadGateway), 0};
	}
	else{
		state_ = StateWriting{BuildCgiResponse(state_cgi.output), 0};
		LOG_DEBUG("cgi: output ready -> writing pid=" + std::to_string(pid)
					+ " fd=" + std::to_string(socket_.fd()),
					srv_id_);
	}

	return instructions;
}
