#pragma once

#include<iostream>
#include <sys/socket.h>
#include <sstream>
#include <memory>

#include "Request.hpp"
#include "Response.hpp"
#include "ClientState.hpp"
#include "Socket.hpp"
#include "PollableFileDescriptor.hpp"
#include "Logger.hpp"

class Client : public ReadFileDescriptor, public WriteFileDescriptor {
	public:
		Client(int fd, const Socket& socket, const Logger& logger);

		// Override the afterRead member function to be able to cut off bad actors,
		// override the readingDone function to build and construct requests

		// Legacy

		// Client(int socket);
		~Client();
		ClientState & getState();
		void receive();
		void sendResponse();
		// void readSocket();

	private:
		Logger        l;
		int           fd = -1;
		const Socket& socket;

		int							m_socket;
		std::shared_ptr<Request> 	m_request;
		std::shared_ptr<Response>	m_response;
		ClientState					m_state;
};
