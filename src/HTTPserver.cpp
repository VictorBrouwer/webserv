#include"HTTPserver.hpp"

HTTPServer::HTTPServer(std::string ip_address, int port) : m_ip_address(ip_address), m_port(port), m_listening_socketAddress_len(sizeof(m_listening_socketAddress))
{
	m_listening_socketAddress.sin_family = AF_INET;
	m_listening_socketAddress.sin_port = htons(m_port);
	m_listening_socketAddress.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
	// m_listening_socketAddress.sin_addr.s_addr = inet_addr(m_ip_address.c_str());

	if (startServer() != 0)
	{
		std::ostringstream ss;
		ss << "Failed to start server with PORT: " << ntohs(m_listening_socketAddress.sin_port);
		log(ss.str());
	}
}

HTTPServer::~HTTPServer()
{
	closeServer();
}

int HTTPServer::startServer()
{
	m_listening_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_listening_socket < 0)
	{
		exitWithError("Cannot create socket");
		return 1;
	}
	int reuse = 1;
	if (setsockopt(m_listening_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
			close(m_listening_socket);
		}

	if (bind(m_listening_socket, (sockaddr *)&m_listening_socketAddress, m_listening_socketAddress_len) < 0)
	{
		exitWithError("Cannot connect socket to address");
		return 1;
	}

	return 0;
}

void HTTPServer::closeServer()
{
	close(m_listening_socket);
	close(m_client_socket);
	exit(0);
}

void HTTPServer::startListen()
{
	if (listen(m_listening_socket, 20) < 0)
		exitWithError("Socket listen failed");

	std::ostringstream ss;
	ss << "\n*** Listening on ADDRESS: " << inet_ntoa(m_listening_socketAddress.sin_addr) << " PORT: " << ntohs(m_listening_socketAddress.sin_port) << " ***\n\n";
	log(Color::White, ss.str());
}

void HTTPServer::startPolling()
{
	this->m_poll.AddPollFd(m_listening_socket, POLLIN);
	std::shared_ptr<Server> server = std::make_shared<Server>(this->m_listening_socket);
	this->m_serverMap.insert({this->m_listening_socket, server});
	while (true)
	{
		log(Color::White ,"====== Waiting for a new event ======\n\n\n");
		pollfd *poll_fds = this->m_poll.getPollFDs().data();
		size_t	num_poll_fds = this->m_poll.getPollFDs().size();
		int num_events = poll(poll_fds, num_poll_fds, -1); // should clarify max wait time now it is set to wait forever(-1)
		if (num_events < 0)
			exitWithError("poll failed");
		if (num_events == 0)
			exitWithError("poll timed out");
		std::cout << "number of events = " << num_events << std::endl;
		for (size_t i = 0; i < num_poll_fds; i++)
		{
			if (poll_fds[i].revents == 0)
				continue;
			int Event_fd = poll_fds[i].fd;
			std::cout << "fd = " << Event_fd << " revent = " << poll_fds[i].revents << std::endl;
			if (this->m_serverMap.find(Event_fd) != this->m_serverMap.end())
				acceptConnection();
			if (this->m_clientMap.find(Event_fd) != this->m_clientMap.end())
				this->HandleActiveClient(this->m_poll.getPollFDs()[i]);
		}
		updatePoll();
	}
}

void HTTPServer::acceptConnection()
{
	m_client_socket = accept(m_listening_socket, (sockaddr *)&m_listening_socketAddress, &m_listening_socketAddress_len);
	if (m_client_socket < 0)
	{
		std::ostringstream ss;
		ss << "Server failed to accept incoming connection from ADDRESS: " << inet_ntoa(m_listening_socketAddress.sin_addr) << "; PORT: " << ntohs(m_listening_socketAddress.sin_port);
		exitWithError(ss.str());
	}
	else
	{
		log(Color::White, "====== accepted a new connection ======\n");
		auto c {std::make_shared<Client>(m_client_socket)};
		this->m_clientMap.insert(std::make_pair(this->m_client_socket, c));
		this->m_poll.AddPollFd(m_client_socket, POLLIN);
	}
}

void HTTPServer::HandleActiveClient(pollfd poll_fd) // still needs work
{
	std::cout << "client update" << std::endl;
	ClientState state = this->m_clientMap.at(poll_fd.fd)->getState();
	switch (poll_fd.revents)
	{
	case POLLIN:
		this->m_clientMap.at(poll_fd.fd)->receive();
		break;
	case POLLOUT:
		if (state == ClientState::READY_TO_SEND)
			this->m_clientMap.at(poll_fd.fd)->sendResponse();
		break;
	default:
		break;
	}
}

void HTTPServer::updatePoll()
{
	for (const auto& pair : m_clientMap)
	{
		int socket = pair.first;
        std::shared_ptr<Client> client = pair.second;
		if (client->getState() == ClientState::READY_TO_SEND)
			m_poll.getPollFDs().at(socket).events = POLLOUT;
		if (client->getState() == ClientState::SENDING_DONE)
			m_poll.getPollFDs().at(socket).events = POLLIN;
	}
}
