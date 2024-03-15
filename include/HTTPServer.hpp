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
#include "HelperFuncs.hpp"
#include "Configuration.hpp"
#include "Logger.hpp"
#include "ConfigShared.hpp"

class HTTPServer : public ConfigShared {
	public:
		HTTPServer(std::string ip_address, int port);
		HTTPServer(Configuration &config, const Logger& l);
		~HTTPServer();
		void startListen();
		void startPolling();

	private:
		std::string        m_ip_address;
		int                m_port;
		int                m_listening_socket;
		int                m_client_socket;
		struct sockaddr_in m_listening_socketAddress;
		unsigned int       m_listening_socketAddress_len;
		std::string        m_serverMessage;
		Poll	           m_poll;

		std::unordered_map<int, std::shared_ptr<Client>> m_clientMap;
		std::unordered_map<int, std::shared_ptr<Server>> m_serverMap;

		std::vector<Client> m_clientVector;
		std::vector<Server> m_serverVector;

		Logger l;
		std::vector<Server> servers;

		int startServer();
		void closeServer();
		void acceptConnection();
		void HandleActiveClient(int i);
		// void HandleActiveClient(pollfd poll_fd);
		void updatePoll();
		void handleEvent(int Event_fd, int i, pollfd *poll_fds);
		// std::string buildResponse();
		// void sendResponse(int fd);
};
