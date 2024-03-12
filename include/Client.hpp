#pragma once

#include<iostream>
#include <sys/socket.h>
#include <sstream>

#include "Request.hpp"
#include "Response.hpp"
#include "ClientState.hpp"


class Client
{
public:
	Client(int socket);
	~Client();
	ClientState & getState();
	void receive();
	// void send();
	void readSocket();
private:
	int				m_socket;
	Request 		m_request;
	Response 		m_response;
	ClientState		m_state;
};
