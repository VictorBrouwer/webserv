#if !defined(CLIENT_HPP)
#define CLIENT_HPP

#include<iostream>
#include <sys/socket.h>

#include"Request.hpp"
#include "ClientState.hpp"

class Client
{
public:
	Client(int socket);
	~Client();
	void receive();
	void createResponse();
	void send();
	void readSocket();
private:
	Request 		m_request;
	int				m_socket;
	ClientState		m_state;
};


#endif // CLIENT_HPP
