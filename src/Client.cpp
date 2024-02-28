#include"Client.hpp"
#include"HelperFuncs.hpp"

// const int BUFFER_SIZE = 30720;

Client::Client(int socket) : m_socket(socket)
{
	std::cout << "Client created\n";
}

Client::~Client()
{
}

ClientState & Client::getState()
{
	return this->m_state;
}

void	Client::sendResponse()
{
}

void	Client::receive()
{
	m_state = m_request.readFromClient(m_socket);
	// m_state is either loading or reading_done
	// if client state is loading, the poll event should remain POLLIN
	// if client statis done_reading, a response should be created and then 
	// the poll event should be set to pollout
	// pollout basically tells you that the sending will succeed
	if (m_state == ClientState::LOADING)
		return ;
	else if (m_state == ClientState::READING_DONE)
	{
		log("reading done!\n", Color::Green);
		log(Color::Yellow, m_request.Get_Request());
		m_response = Response(m_request);
		m_response.createResponse();
	}
	else
	{
		std::ostringstream ss;
		ss << "error occured while receiving at fd: " << m_socket << std::endl;
		log(ss.str(), Color::Red);
	}
}
