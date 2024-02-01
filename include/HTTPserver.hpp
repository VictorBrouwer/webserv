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

#include "Poll.hpp"
#include "Client.hpp"

class HTTPServer
{
public:
	HTTPServer(std::string ip_address, int port);
	~HTTPServer();
	void startListen();
	void startPolling();

private:
	std::string m_ip_address;
	int m_port;
	int m_socket;
	int m_new_socket;
	struct sockaddr_in m_socketAddress;
	unsigned int m_socketAddress_len;
	std::string m_serverMessage;
	Poll	m_poll;

	// std::unordered_map<int, Client> ClientMap;
	// std::unordered_map<int, Client> ServerMap;

	int startServer();
	void closeServer();
	void acceptConnection(int &new_socket);
	std::string buildResponse();
	void sendResponse(int fd);
	void HandleActiveClient(struct pollfd curr);
};

#endif // HTTP_SERVER_HPP
