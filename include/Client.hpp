#if !defined(CLIENT_HPP)
#define CLIENT_HPP

#include"Request.hpp"

class Client
{
public:
	Client(/* args */);
	~Client();
private:
	Request m_request;
	void parseRequest(struct pollfd curr, char *buffer, int bytesReceived);
};


#endif // CLIENT_HPP
