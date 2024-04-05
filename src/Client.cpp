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
	std::string stream_contents = std::move(this->read_buffer).str();

	std::size_t header_boundary = stream_contents.find("\r\n\r\n");

	if (header_boundary != std::string::npos && this->bytes_read > header_limit) {
		l.log("Max header size exceeded, cutting off the connection", L_Error);
		this->setReadFDStatus(FD_ERROR);

		// Set up error Response and hit send
	}

	if (this->bytes_read > body_limit) {
		l.log("Max body size exceeded, cutting off the connection.", L_Warning);
		this->setReadFDStatus(FD_ERROR);

		// Set up error Response and hit send
	}

	this->read_buffer.str(std::move(stream_contents));
}

// If we have to keepalive, keep the file descriptor open
void Client::readingDone( void ) {
	// this->m_request = std::shared_ptr(new Request(this->read_buffer));
}

// Legacy

ClientState & Client::getState()
{
	return this->m_state;
}

void	Client::sendResponse()
{
	int bytes_sent;

	// write(this->m_socket, m_response.getResponse().c_str(), m_response.getResponse().size());
	// log(std::string("sending response: " + m_response->getResponse()), L_Info);
	bytes_sent = send(this->m_socket, m_response->getResponse().c_str(), m_response->getResponse().size(), 0);
	if (bytes_sent < 0)
	{
		m_state = ClientState::ERROR;
		return;
	}
	m_total_bytes_sent += bytes_sent;
	if (m_total_bytes_sent == this->m_response->getResponse().length())
	{
		if(m_request->Get_Keep_Alive() == true)
		{
			m_request.reset(new Request);
			m_response.reset(new Response(m_request));
			m_state = ClientState::KEEP_ALIVE;
			m_total_bytes_sent = 0;
		}
		else
			m_state = ClientState::REMOVE_CONNECTION;
	}
	else if (m_total_bytes_sent < this->m_response->getResponse().length())
		m_state = ClientState::SENDING;
}

void	Client::receive(std::vector<Server> &servers)
{
	m_state = m_request->readFromClient(m_socket);
	log("\n" + m_request->Get_Request(), L_Info);
	// m_state is either loading or reading_done
	// if client state is loading, the poll event should remain POLLIN
	// if client statis done_reading, a response should be created and then
	// the poll event should be set to pollout
	// pollout basically tells you that the sending will succeed
	switch (m_state)
	{
	case ClientState::ERROR:
		break;
	case ClientState::LOADING:
		break;
	case ClientState::READING_DONE:
		this->extractServer(servers);
		this->checkRequestSyntax(m_request->Get_Request());
		m_request->handleLocation(m_server);
		m_response->createResponse(m_server);
		m_state = ClientState::READY_TO_SEND; // maybe set this somewhere else
		break;
	default:
		break;
	}
}

void Client::checkRequestSyntax(const std::string& request)
{
   std::istringstream iss(request);
   std::vector<std::string> lines;
   std::string line;
   while (std::getline(iss, line))
	   lines.push_back(line);
	// Check the minimum number of lines
   if (lines.size() < 3)
		throw std::runtime_error("invalid HTTP-request");
	const std::string& request_line = lines[0];
	std::istringstream request_line_stream(request_line);
	std::string method, path, http_version;
	request_line_stream >> method >> path >> http_version;
	if (path[0] != '/')
		throw std::runtime_error("invalid HTTP-request");
	if (http_version != "HTTP/1.1")
		throw std::runtime_error("invalid HTTP-request");
}

void	Client::extractServer(std::vector<Server> &servers)
{
	for (auto server : servers)
	{
		for (const auto& servername : server.getServerNames())
		{
			if (servername == this->m_request->extractHostPort(HostPort::HOST))
				m_server = &server;
		}
	}
	if (!m_server)
		m_server = &(*(servers.begin()));
		// m_server = servers->data();
}
