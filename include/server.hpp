#if !defined(SERVER_HPP)
#define SERVER_HPP

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
#include <poll.hpp>

class Server
{
public:
	Server(std::string ip_address, int port);
	~Server();
	void startListen();

private:
	std::string m_ip_address;
	int m_port;
	int m_socket;
	int m_new_socket;
	// long m_incomingMessage;
	struct sockaddr_in m_socketAddress;
	unsigned int m_socketAddress_len;
	std::string m_serverMessage;
	Poll	m_poll;

	int startServer();
	void closeServer();
	void acceptConnection(int &new_socket);
	std::string buildResponse();
	void sendResponse();
	void HandleActiveClient(struct pollfd curr);
};

#endif // SERVER_HPP
