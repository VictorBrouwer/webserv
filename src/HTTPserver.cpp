#include <algorithm>

#include "HTTPServer.hpp"
#include "Configuration.hpp"
#include "Directive.hpp"
#include "Socket.hpp"

HTTPServer::HTTPServer(Configuration &config, const Logger& logger) : ConfigShared(), l("HTTPServer", logger.getLogLevel())
{
	m_listening_socketAddress.sin_family = AF_INET;
	m_listening_socketAddress.sin_port = htons(m_port); // m_port kan waarscijnlijk verwijderd worden
	m_listening_socketAddress.sin_addr.s_addr = INADDR_ANY;
	try {
		l.log("Constructing HTTPServer.", L_Info);

		l.log("Checking for http directive");
		const Directive& http_directive = config.getHttpDirective();

		this->applySharedDirectives(http_directive.getSubdirectives(), l);

		std::vector<Directive>::const_iterator it  = http_directive.getSubdirectivesIterator();
		std::vector<Directive>::const_iterator end = http_directive.getSubdirectivesEnd();

		l.log("Setting up virtual servers.", L_Info);
		while (it != end) {
			if (*it == "server")
				this->servers.push_back(Server(*it, (ConfigShared*) this, l));
			++it;
		}
		l.log("Setting up sockets.", L_Info);
		this->setupSockets();
	}
	catch(const std::exception& e) {
		l.log(e.what(), L_Error);
		throw; // Rethrow to catch in main
	}
}

HTTPServer::~HTTPServer() {
	// closeServer();
}

std::vector<Server>::iterator HTTPServer::getServerMutableIterator( void ) {
	return this->servers.begin();
}

std::vector<Server>::iterator HTTPServer::getServerMutableEnd( void ) {
	return this->servers.end();
}

std::vector<Socket>::const_iterator HTTPServer::getSocketIterator( void ) const {
	return this->sockets.begin();
}

std::vector<Socket>::const_iterator HTTPServer::getSocketEnd( void ) const {
	return this->sockets.end();
}

// Collects all sockets to open for the virtual servers and binds them
// to their addresses. Populates the sockets vector in HTTPServer.
void HTTPServer::setupSockets( void ) {
	l.log("Collecting required sockets.");
	std::map<int, std::vector<std::string>> sockets_requested;

	auto start = this->getServerMutableIterator();
	auto end   = this->getServerMutableEnd();
	auto it    = start;

	// Collect all listen directives in our map, sorted by port
	while (it != end) {
		auto listens = it->getListens();
		std::for_each(
			listens.begin(), listens.end(),
			[&](const std::pair<std::string, int>& p) {
				sockets_requested[p.second].push_back(p.first);
			}
		);
		++it;
	}

	std::vector<std::pair<std::string, int>> sockets_to_open;

	// Check which sockets need to be opened for each port
	// If we have 0.0.0.0 or *, open that, else open each unique one individually
	std::for_each(
		sockets_requested.begin(), sockets_requested.end(),
		[&](const std::pair<const int, std::vector<std::string>>& p) {
			if (std::find(p.second.begin(), p.second.end(), "*") != p.second.end() ||
				std::find(p.second.begin(), p.second.end(), "0.0.0.0") != p.second.end()) {
				sockets_to_open.push_back({"0.0.0.0", p.first});
			} else {
				std::for_each(p.second.begin(), p.second.end(), [&](const std::string& interface) {
					sockets_to_open.push_back({interface, p.first});
				});
			}
		}
	);

	l.log("Done. Amount of sockets to open: " + std::to_string(sockets_to_open.size()));
	l.log("Opening sockets...");
	std::for_each(sockets_to_open.begin(), sockets_to_open.end(), [&](const std::pair<std::string, int>& s) {
		this->sockets.emplace_back(s.first, s.second, l);
	});

	l.log("Done. Sockets are ready for listening.");
}

void HTTPServer::startListening( void ) const {
	l.log("Calling startListening() on all sockets.");
	std::for_each(this->getSocketIterator(), this->getSocketEnd(), [](const Socket& s) {
		s.startListening();
	});
}

void HTTPServer::closeServer()
{
}

void HTTPServer::addSocketsToPoll()
{
	for (const auto & socket :this->sockets)
	{
		this->m_poll.AddPollFd(socket.getFileDescriptor(), POLLIN);
		log(std::string("added socket: " + std::to_string(socket.getFileDescriptor()) + " to Poll"));
	}
}

void HTTPServer::startPolling()
{
	addSocketsToPoll();
	while (true)
	{
		log("====== Waiting for a new event ======\n\n\n");
		pollfd *poll_fds = this->m_poll.getPollFDs().data();
		size_t	num_poll_fds = this->m_poll.getPollFDs().size();
		int num_events = poll(poll_fds, num_poll_fds, -1); // should clarify max wait time. now it is set to wait forever(-1)
		if (num_events < 0)
			exitWithError("poll failed");
		if (num_events == 0)
			exitWithError("poll timed out");
		std::cout << "number of events = " << num_events << std::endl;
		for (size_t i = 0; i < num_poll_fds; i++)
		{
			int Event_fd = poll_fds[i].fd;
			if (poll_fds[i].revents != 0)
			{
				handleEvent(Event_fd, i, poll_fds);
				num_events--;
			}
			if (num_events <= 0)
				break;
		}
	}
}

std::string formatRevents(const struct pollfd& poll_fd) {
    std::string result = ", revents:";
    result += (poll_fd.revents & POLLIN) ? " POLLIN" : "";
    result += (poll_fd.revents & POLLOUT) ? " POLLOUT" : "";
    result += (poll_fd.revents & POLLHUP) ? " POLLHUP" : "";
    result += (poll_fd.revents & POLLNVAL) ? " POLLNVAL" : "";
    result += (poll_fd.revents & POLLPRI) ? " POLLPRI" : "";
    result += (poll_fd.revents & POLLRDBAND) ? " POLLRDBAND" : "";
    result += (poll_fd.revents & POLLRDNORM) ? " POLLRDNORM" : "";
    result += (poll_fd.revents & POLLWRBAND) ? " POLLWRBAND" : "";
    result += (poll_fd.revents & POLLWRNORM) ? " POLLWRNORM" : "";
    result += (poll_fd.revents & POLLERR) ? " POLLERR" : "";
    return result;
}

void HTTPServer::handleEvent(int Event_fd, int i, pollfd *poll_fds)
{
	log(std::string("fd = " + std::to_string(Event_fd) + formatRevents(poll_fds[i])), Color::Blue);
	try
	{
		this->m_poll.checkErrors(poll_fds[i].revents);
	}
	catch(const std::exception& e)
	{
		std::string error_message = std::string(e.what()) + " at fd: " + std::to_string(Event_fd);
		log(error_message, Color::Red);
		this->m_poll.RemovePollFd(Event_fd);
		this->m_clientMap.erase(Event_fd);
	}
	for (const auto & socket : this->sockets)
	{
		if (socket.getFileDescriptor() == Event_fd)
			acceptConnection(Event_fd);
	}
	if (this->m_clientMap.find(Event_fd) != this->m_clientMap.end())
		this->HandleActiveClient(i);
}

void HTTPServer::acceptConnection(int Event_fd)
{
	m_client_socket = accept(Event_fd, (sockaddr *)&m_listening_socketAddress, &m_listening_socketAddress_len);
	if (m_client_socket < 0)
	{
		std::ostringstream ss;
		ss << "Server failed to accept incoming connection from ADDRESS: " << inet_ntoa(m_listening_socketAddress.sin_addr) << "; PORT: " << ntohs(m_listening_socketAddress.sin_port);
		log(ss.str(), Color::Red); // also remove 
	}
	else
	{
		log("====== accepted a new connection ======\n");
		auto c {std::make_shared<Client>(m_client_socket)};
		this->m_clientMap.insert(std::make_pair(this->m_client_socket, c));
		this->m_poll.AddPollFd(m_client_socket, POLLIN);
	}
}

void HTTPServer::HandleActiveClient(int i) // still needs work
{
	pollfd poll_fd = this->m_poll.getPollFDs()[i];
	std::cout << "client update" << std::endl;
	ClientState state = this->m_clientMap.at(poll_fd.fd)->getState();
	std::shared_ptr<Client> active_client = this->m_clientMap.at(poll_fd.fd);
	switch (poll_fd.revents)
	{
	case POLLIN:
		active_client->receive(servers);
		state = active_client->getState();
		break;
	case POLLOUT:
		if (state == ClientState::READY_TO_SEND)
		{
			active_client->sendResponse();
			state = active_client->getState();
		}
		break;
	default:
		break;
	}
	this->updatePoll(state, poll_fd);
}

void HTTPServer::updatePoll(ClientState state, pollfd poll_fd)
{
	switch (state)
	{
	case ClientState::ERROR:
		this->m_poll.RemovePollFd(poll_fd.fd);
		this->m_clientMap.erase(poll_fd.fd);
		log(std::string("removed client with fd: " + std::to_string(poll_fd.fd)), Color::Blue);
		break;
	case ClientState::LOADING:
		this->m_poll.setEvents(poll_fd.fd, POLLIN);
		break;
	case ClientState::READING_DONE: // unsure if this is possible. Also, should you still accept incoming requests at this point?
		this->m_poll.setEvents(poll_fd.fd, POLLIN);
		break;
	case ClientState::READY_TO_SEND:
		this->m_poll.setEvents(poll_fd.fd, POLLOUT);
		break;
	case ClientState::SENDING:
		this->m_poll.setEvents(poll_fd.fd, POLLOUT);
		break;
	case ClientState::KEEP_ALIVE:
		this->m_poll.unsetEvents(poll_fd.fd);
		this->m_poll.setEvents(poll_fd.fd, POLLIN);
		break;
	case ClientState::REMOVE_CONNECTION:
		this->m_poll.RemovePollFd(poll_fd.fd);
		this->m_clientMap.erase(poll_fd.fd);
		log(std::string("removed client with fd: " + std::to_string(poll_fd.fd)), Color::Blue);
		break;
	default:
		break;
	}
}
