#pragma once

#include<iostream>
#include <sys/socket.h>
#include <sstream>
#include <memory>

#include "Request.hpp"
#include "Response.hpp"
#include "ClientState.hpp"
// #include "Socket.hpp"
#include "PollableFileDescriptor.hpp"
#include "Logger.hpp"

class Socket;
class Server;

class Client : public ReadFileDescriptor, public WriteFileDescriptor {
	public:
		Client(
			int fd, sockaddr address, socklen_t addr_len,
			const Socket& socket, const Logger& logger
		);

		Client(const Client& src);
		Client(Client&& to_move);
		Client& operator=(Client& src);
		Client& operator=(const Client& src);

		~Client();

		std::shared_ptr<Request>& getRequest( void );
		std::shared_ptr<Response>& getResponse( void );

		void addToWriteBuffer(const std::string& str);

		// Legacy

		ClientState & getState();
		void receive(std::vector<Server> &servers);
		void extractServer(std::vector<Server> &servers);
		void extractServer( void );
		void sendResponse();
		void checkRequestSyntax(const std::string& request);
	private:
		// Override the afterRead member function to be able to cut off bad actors
		void afterRead( void );
		void afterReadDuringHeaders(std::string& stream_contentst);
		void afterReadDuringBody(std::string& stream_contents);
		// Override the readingDone function to build and construct requests
		void readingDone( void );

		// Override the writingDone member function to be able to keepalive
		void writingDone( void );

		// Override the doRead function because we do things differently
		// when we receive a read of size zero
		ssize_t doRead( void );

		Logger        l;
		const Socket& socket;

		int       fd = -1;
		sockaddr  address;
		socklen_t address_length;
		bool	  reading_body = false;
		bool	  chunked_request = false;

		int							m_socket;
		std::shared_ptr<Request> 	m_request;
		std::shared_ptr<Response>	m_response;
		ClientState					m_state;
		size_t						m_total_bytes_sent;
		Server 						*m_server = nullptr;

		std::size_t header_limit = 50000;
		std::size_t body_limit = 10000000;
};
