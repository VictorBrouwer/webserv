#pragma once

#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <fstream>
#include <poll.h>
#include <unordered_map>
#include <vector>
#include <memory>

#include "Poll.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Socket.hpp"
#include "HelperFuncs.hpp"
#include "Configuration.hpp"
#include "Logger.hpp"
#include "ConfigShared.hpp"

class HTTPServer : public ConfigShared {
	public:
		HTTPServer(Configuration &config, const Logger& l);

		~HTTPServer();

		std::vector<Server>::iterator getServerMutableIterator( void );
		std::vector<Server>::iterator getServerMutableEnd( void );

		std::vector<Socket>::const_iterator getSocketIterator( void ) const;
		std::vector<Socket>::const_iterator getSocketEnd( void ) const;

		void startListening( void ) const;

		// Legacy
		void startPolling();

	private:
		// Legacy
		int                m_port;
		int                m_client_socket;
		struct sockaddr_in m_listening_socketAddress;
		unsigned int       m_listening_socketAddress_len;
		std::string        m_serverMessage;
		Poll	           m_poll;

		std::unordered_map<int, std::shared_ptr<Client>> m_clientMap;

		Logger l;

		// A Socket is a file descriptor on which we listen for new clients.
		// It is a separate entity from servers because multiple servers
		// may listen on the same socket, or a different subset of them.
		std::vector<Socket> sockets;

		// A Server is a virtual server registered in the config that listens
		// on at least one of our Sockets and describes how it wants to answer
		// requests. When a Request comes in, it's matched to a Server, which
		// constructs a Response for it.
		std::vector<Server> servers;

		// A Client is a currently open connection on which we might receive
		// Requests, which we need to answer with a Response.
		std::vector<Client> clients;

		std::map<int, std::pair<std::string, int>> socket_map;

		void closeServer();
		void acceptConnection(int Event_fd);
		void HandleActiveClient(int i);
		// void HandleActiveClient(pollfd poll_fd);
		void addSocketsToPoll();
		void updatePoll(ClientState state, pollfd poll_fd);
		void handleEvent(int Event_fd, int i, pollfd *poll_fds);
		// std::string buildResponse();
		// void sendResponse(int fd);

		void setupSockets( void );
};
