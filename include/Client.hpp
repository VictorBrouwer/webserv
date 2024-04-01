#pragma once


#include<iostream>
#include <sys/socket.h>
#include <sstream>
#include <memory>

#include "Request.hpp"
#include "Response.hpp"
#include "ClientState.hpp"
#include "Server.hpp"


class Client
{
public:
	Client(int socket);
	~Client();
	ClientState & getState();
	void receive(std::vector<Server> &servers);
	void extractServer(std::vector<Server> &servers);
	void sendResponse();
	void readSocket();
	void checkRequestSyntax(const std::string& request);
private:
	int							m_socket;
	std::shared_ptr<Request> 	m_request;
	std::shared_ptr<Response>	m_response;
	ClientState					m_state;
	size_t						m_total_bytes_sent;
	Server 						*m_server;
};
