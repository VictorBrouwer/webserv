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

class Client : public ReadFileDescriptor, public WriteFileDescriptor {
	public:
		Client(
			int fd, sockaddr address, socklen_t addr_len,
			const Socket& socket, const Logger& logger
		);

		Client(const Client& src);
		Client(Client&& to_move);

		// Legacy

		// Client(int socket);
		~Client();
		ClientState & getState();
		void receive();
		void sendResponse();
		// void readSocket();

	private:
		// Override the afterRead member function to be able to cut off bad actors
		void afterRead( void );
		// Override the readingDone function to build and construct requests
		void readingDone( void );

		Logger        l;
		const Socket& socket;

		int       fd = -1;
		sockaddr  address;
		socklen_t address_length;

		int							m_socket;
		std::shared_ptr<Request> 	m_request;
		std::shared_ptr<Response>	m_response;
		ClientState					m_state;
};
