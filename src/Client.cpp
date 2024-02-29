#include"Client.hpp"
#include"HelperFuncs.hpp"

// const int BUFFER_SIZE = 30720;

Client::Client(int socket) : m_socket(socket), m_response(this->m_request)
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
	case ClientState::READING_DONE:
		m_response.createResponse();
		write(this->m_socket, m_response.getResponse().c_str(), m_response.getResponse().size());
		break;
	
	default:
		break;
	}
}