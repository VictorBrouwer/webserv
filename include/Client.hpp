#if !defined(CLIENT_HPP)
#define CLIENT_HPP

#include<iostream>
#include <sys/socket.h>
#include <sstream>
#include <memory>

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
	void sendResponse();
	void readSocket();
private:
	int							m_socket;
	std::shared_ptr<Request> 	m_request;
	std::shared_ptr<Response>	m_response;
	ClientState					m_state;
	size_t						m_total_bytes_sent;
};


#endif // CLIENT_HPP
