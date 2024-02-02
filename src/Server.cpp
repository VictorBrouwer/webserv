#include"Server.hpp"
#include"HelperFuncs.hpp"

Server::Server(int socket) : m_socket(socket)
{
	log("Server created\n");
}

Server::~Server()
{
}
