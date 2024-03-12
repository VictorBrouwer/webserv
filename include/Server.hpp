#pragma once

#include "Directive.hpp"
#include "Location.hpp"

class Server
{
public:
	Server(int socket);
	Server(const Directive &server_directive);
	~Server();
private:
	// Original member variables
	int	m_socket;

	// Config-parsed member variables

	std::vector<Location> _locations;
};
