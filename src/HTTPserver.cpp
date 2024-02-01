#include"HTTPserver.hpp"

const int BUFFER_SIZE = 30720;

void log(const std::string &message)
{
	std::cout << message << std::endl;
}

void exitWithError(const std::string &errorMessage)
{
	log("ERROR: " + errorMessage);
	exit(1);
}

HTTPServer::HTTPServer(std::string ip_address, int port) : m_ip_address(ip_address), m_port(port), m_socketAddress_len(sizeof(m_socketAddress))
{
	m_socketAddress.sin_family = AF_INET;
	m_socketAddress.sin_port = htons(m_port);
	m_socketAddress.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
	// m_socketAddress.sin_addr.s_addr = inet_addr(m_ip_address.c_str());

	if (startServer() != 0)
	{
		std::ostringstream ss;
		ss << "Failed to start server with PORT: " << ntohs(m_socketAddress.sin_port);
		log(ss.str());
	}
}

HTTPServer::~HTTPServer()
{
	closeServer();
}

int HTTPServer::startServer()
{
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket < 0)
	{
		exitWithError("Cannot create socket");
		return 1;
	}
	int reuse = 1;
	if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
			close(m_socket);
		}

	if (bind(m_socket, (sockaddr *)&m_socketAddress, m_socketAddress_len) < 0)
	{
		exitWithError("Cannot connect socket to address");
		return 1;
	}

	return 0;
}

void HTTPServer::closeServer()
{
	close(m_socket);
	close(m_new_socket);
	exit(0);
}

void HTTPServer::startListen()
{
	if (listen(m_socket, 20) < 0)
		exitWithError("Socket listen failed");

	std::ostringstream ss;
	ss << "\n*** Listening on ADDRESS: " << inet_ntoa(m_socketAddress.sin_addr) << " PORT: " << ntohs(m_socketAddress.sin_port) << " ***\n\n";
	log(ss.str());
}

void HTTPServer::startPolling()
{
	this->m_poll.AddPollFd(m_socket, POLLIN);
	while (true)
	{
		log("====== Waiting for a new event ======\n\n\n");
		int num_events = poll(this->m_poll.getPollFDs().data(), this->m_poll.getPollFDs().size(), -1); // should clarify max wait time now it is set to wait forever(-1)
		if (num_events < 0)
			exitWithError("poll failed");
		if (num_events == 0)
			exitWithError("poll timed out");
		std::cout << "number of events = " << num_events << std::endl;
		for (size_t i = 0; i < this->m_poll.getPollFDs().size() ; i++)
		{
			std::cout << "fd = " << i << " revent = " << this->m_poll.getPollFDs()[i].revents << std::endl;
			if (this->m_poll.getPollFDs()[i].revents & POLLIN)
			{
				if (i == 0)
				{
					std::cout << "new incoming connection" << std::endl;
					acceptConnection(m_new_socket);
					this->m_poll.AddPollFd(m_new_socket, POLLIN);
				}
				else
				{
					std::cout << "client update" << std::endl;
					this->HandleActiveClient(this->m_poll.getPollFDs()[i]);
				}
			}
		}

		// std::ostringstream ss;
		// ss << "------ Received Request from client ------\n\n";
		// log(ss.str());

		// // sendResponse();

		// close(m_new_socket);
	}
}

void HTTPServer::acceptConnection(int &new_socket)
{
	new_socket = accept(m_socket, (sockaddr *)&m_socketAddress, &m_socketAddress_len);
	if (new_socket < 0)
	{
		std::ostringstream ss;
		ss << "Server failed to accept incoming connection from ADDRESS: " << inet_ntoa(m_socketAddress.sin_addr) << "; PORT: " << ntohs(m_socketAddress.sin_port);
		exitWithError(ss.str());
	}
	else
		log("====== accepted a new connection ======\n");
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

void HTTPServer::HandleActiveClient(struct pollfd curr)
{
	int bytesReceived;
	char buffer[BUFFER_SIZE] = {0};
	switch (curr.revents)
	{
	case POLLIN:
		log("read case");
		bytesReceived = recv(curr.fd, buffer, BUFFER_SIZE, 0);
		if (bytesReceived < 0)
			exitWithError("Failed to read bytes from client socket connection");
		log(buffer);
		break;
	case POLLOUT:
		log("write case");
		this->sendResponse(curr.fd);
	default:
		break;
	}
}

// void Server::parseRequest(struct pollfd curr, char *buffer, int bytesReceived)
// {

// }