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

void	Client::receive()
{
	m_state = m_request.readFromClient(m_socket);
	// if client state is loading, the poll event should remain loading
	// if client statis done_reading, a response should be created and then 
	// the poll event should be set to pollout
	// pollout basically tells you that the sending will succeed
	switch (m_state)
	{
	case ClientState::LOADING:
		
		break;
	
	default:
		break;
	}
}


// void	Client::readSocket()
// {
// 	log("reading from socker");
// 	int bytesReceived;
// 	char buffer[BUFFER_SIZE] = {0};
// 	bytesReceived = recv(this->m_socket, buffer, BUFFER_SIZE, 0);
// 	if (bytesReceived < 0)
// 		exitWithError("Failed to read bytes from client socket connection");
// 	log(buffer);
// }