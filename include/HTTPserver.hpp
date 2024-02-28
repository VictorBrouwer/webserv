#if !defined(HTTP_SERVER_HPP)
#define HTTP_SERVER_HPP

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

class HTTPServer
{
public:
	HTTPServer(std::string ip_address, int port);
	HTTPServer(const Configuration &config);
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

	int startServer();
	void closeServer();
	void acceptConnection();
	void HandleActiveClient(pollfd poll_fd);
	std::string buildResponse();
	void sendResponse(int fd);
};

#endif // HTTP_SERVER_HPP
