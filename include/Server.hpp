#if !defined(SERVER_HPP)
#define SERVER_HPP
#include "Directive.hpp"

class Server
{
public:
	Server(int socket);
	Server(const Directive &server_directive);
	~Server();
private:
	int	m_socket;
};

#endif // SERVER_HPP
