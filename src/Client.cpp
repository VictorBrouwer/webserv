#include"Client.hpp"
#include"HelperFuncs.hpp"

// const int BUFFER_SIZE = 30720;

Client::Client(int socket) : m_socket(socket), m_request(std::make_shared<Request>())
{
	Response response(this->m_request);
	m_response = std::make_shared<Response>(response);
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
	// write(this->m_socket, m_response.getResponse().c_str(), m_response.getResponse().size());
	log(std::string("sending response: " + m_response->getResponse()), L_Warning);
	if (send(this->m_socket, m_response->getResponse().c_str(), m_response->getResponse().size(), 0) > 0)
	{
		this->m_request.reset(new Request());
		this->m_response.reset(new Response(m_request));
		m_state = ClientState::SENDING_DONE;
	}
	else
		m_state = ClientState::ERROR;
}

void	Client::receive()
{
	m_state = m_request->readFromClient(m_socket);
	log(m_request->Get_Request(), Color::Yellow);
	// m_state is either loading or reading_done
	// if client state is loading, the poll event should remain POLLIN
	// if client statis done_reading, a response should be created and then 
	// the poll event should be set to pollout
	// pollout basically tells you that the sending will succeed
	switch (m_state)
	{
	case ClientState::LOADING:
		break;
	case ClientState::READING_DONE:
		m_response->createResponse();
		m_state = ClientState::READY_TO_SEND; // maybe set this somewhere else
		break;
	default:
		break;
	}
}