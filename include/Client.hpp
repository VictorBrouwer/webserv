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

		// Override to do recv instead of reading, and to cut off if we
		// do not receive a properly formatted request after some time
		void readFromFileDescriptor( void );

		// We have received data and should process it to form a Request
		void readingDone( void );

		// Override to do send instead of writing
		void writeToFileDescriptor( void );

		// Legacy
		// Client(int socket);
		~Client();
		ClientState & getState();
		void receive();
		void sendResponse();
		void readSocket();

	private:
		Logger        l;
		int           fd;
		const Socket& socket;

		int							m_socket;
		std::shared_ptr<Request> 	m_request;
		std::shared_ptr<Response>	m_response;
		ClientState					m_state;
};
