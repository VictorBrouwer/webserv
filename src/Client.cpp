#include"Client.hpp"
#include"HelperFuncs.hpp"

Client::Client(int fd, sockaddr address, socklen_t addr_len, const Socket& socket, const Logger& logger) :
	ReadFileDescriptor(fd), WriteFileDescriptor(fd),
	l(logger), socket(socket) {
	this->l.setDefaultContext("Client");
	this->fd = fd;

	this->address        = address;
	this->address_length = addr_len;

	l.log("Marking client socket as ready for reading");
	this->setReadFDStatus(FD_POLLING);
	(void) this->socket;
}

Client::Client(const Client& src) : ReadFileDescriptor(src.fd), WriteFileDescriptor(src.fd), socket(src.socket) {
	this->fd = src.fd;

	this->l = src.l;
	this->address = src.address;
	this->address_length = src.address_length;
}

Client::Client(Client&& to_move) : ReadFileDescriptor(to_move.fd), WriteFileDescriptor(to_move.fd), socket(to_move.socket) {
	this->fd = to_move.fd;
	to_move.fd = -1;

	this->l = to_move.l;
	this->address = to_move.address;
	this->address_length = to_move.address_length;
}

Client::~Client() {
	if (this->fd != -1)
		close(this->fd);
}

// Check if we have a full request ready or if we've been kept
// reading for too long
void Client::afterRead( void ) {

}

// If we have to keepalive, keep the file descriptor open
void Client::readingDone( void ) {

}

// Legacy

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
