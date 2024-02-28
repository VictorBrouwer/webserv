#include"HTTPserver.hpp"
#include "Configuration.hpp"

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

HTTPServer::HTTPServer(const Configuration &config)
{

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
	log(ss.str());
}

void HTTPServer::startPolling()
{
	this->m_poll.AddPollFd(m_listening_socket, POLLIN);
	std::shared_ptr<Server> server = std::make_shared<Server>(this->m_listening_socket);
	this->m_serverMap.insert({this->m_listening_socket, server});
	while (true)
	{
		log("====== Waiting for a new event ======\n\n\n");
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
		log("====== accepted a new connection ======\n");
		auto c {std::make_shared<Client>(m_client_socket)};
		this->m_clientMap.insert(std::make_pair(this->m_client_socket, c));
		this->m_poll.AddPollFd(m_client_socket, POLLIN);
	}
}

void HTTPServer::HandleActiveClient(pollfd poll_fd)
{
	std::cout << "client update" << std::endl;
	switch (poll_fd.revents)
	{
	case POLLIN:
		this->m_clientMap.at(poll_fd.fd)->receive();
		break;
	case POLLOUT:
		// this->m_clientMap.at(poll_fd.fd)->send();
		break;
	default:
		break;
	}

}

std::string HTTPServer::buildResponse()
{
	// std::ifstream infile;
	// infile.open("/home/vbrouwer/core/webserv/server/test.html", std::ios::in);
	// std::string buffer, str;
	// while (std::getline(infile, buffer))
	// {
	// 	str.append(buffer);
	// }

	std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from your Server :) </p></body></html>";
	std::ostringstream ss;
	ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n"
		<< htmlFile;

	return ss.str();
}

void HTTPServer::sendResponse(int fd)
{
	m_serverMessage = buildResponse();
	long unsigned int bytesSent;
	bytesSent = write(fd, m_serverMessage.c_str(), m_serverMessage.size());
	if (bytesSent == m_serverMessage.size())
		log("------ Server Response sent to client ------\n\n");
	else
		log("Error sending response to client");
}
