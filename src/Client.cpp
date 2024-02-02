#include"Client.hpp"
#include"HelperFuncs.hpp"

const int BUFFER_SIZE = 30720;

Client::Client(int socket) : m_socket(socket)
{
	std::cout << "Client created\n";
}

Client::~Client()
{
}

void	Client::parseRequest()
{
	
}
void	Client::readSocket()
{
	log("reading from socker");
	int bytesReceived;
	char buffer[BUFFER_SIZE] = {0};
	bytesReceived = recv(this->m_socket, buffer, BUFFER_SIZE, 0);
	if (bytesReceived < 0)
		exitWithError("Failed to read bytes from client socket connection");
	log(buffer);
}