#if !defined(SERVER_HPP)
#define SERVER_HPP

class Server
{
public:
	Server(int socket);
	~Server();
private:
	int	m_socket;
};



#endif // SERVER_HPP
