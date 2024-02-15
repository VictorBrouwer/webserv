#if !defined(CLIENT_HPP)
#define CLIENT_HPP

#include<iostream>
#include <sys/socket.h>

#include"Request.hpp"
#include"Response.hpp"
#include "ClientState.hpp"

class Client
{
public:
	Client(int socket);
	~Client();
	ClientState & getState();
	void receive();
	// void createResponse(Request &request);
	void sendResponse();
	void readSocket();
private:
	Request 		m_request;
	Response 		m_response;
	int				m_socket;
	ClientState		m_state;
};


#endif // CLIENT_HPP
