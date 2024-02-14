#if !defined(CLIENT_HPP)
#define CLIENT_HPP

#include<iostream>
#include <sys/socket.h>

#include"Request.hpp"

class Client
{
public:
	Client(int socket);
	~Client();
	void receive();
	void readSocket();
private:
	Request m_request;
	int		m_socket;
};


#endif // CLIENT_HPP
