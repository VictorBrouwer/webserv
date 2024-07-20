#include <cerrno>
#include <cstring>

#include "Socket.hpp"

Socket::Socket(
	const std::string& interface, int port,
	const Logger& logger, std::vector<Client>& client_vector
) : ReadFileDescriptor(-1), l(logger) {
	this->interface     = interface;
	this->port          = port;
	this->client_vector = &client_vector;

	l.setDefaultContext("Socket " + this->toString());
	l.log("Setting up socket for " + interface + ":" + std::to_string(port));
	this->socket_struct.sin_family = AF_INET;
	this->socket_struct.sin_port   = htons(port);
	if (this->interface == "0.0.0.0")
		this->socket_struct.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
	else
		// This line was commented out before, is it all good?
		this->socket_struct.sin_addr.s_addr = inet_addr(interface.c_str());

	l.log("Opening and binding socket.");
	this->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->fd < 0) {
		throw Socket::Exception(*this, "Could not open socket: " + std::string(std::strerror(errno)));
	}

	this->setReadFileDescriptor(this->fd);

	const int reuse = 1;
	if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) != 0) {
		throw Socket::Exception(*this, "Could not configure socket options: " + std::string(std::strerror(errno)));
	}

	if (bind(this->fd, (sockaddr *)&(this->socket_struct), sizeof(this->socket_struct)) != 0) {
		throw Socket::Exception(*this, "Could not bind socket to address: " + std::string(std::strerror(errno)));
	}
}

Socket::~Socket() {
	// if (this->fd > 0) {
	// 	close(this->fd);
	// }
}

const std::string& Socket::getInterface( void ) const {
	return this->interface;
}

int Socket::getPort( void ) const {
	return this->port;
}

int Socket::getFileDescriptor( void ) const {
	return this->fd;
}

std::string Socket::toString( void ) const {
	return this->interface + ":" + std::to_string(this->port);
}

void Socket::startListening( void ) {
	if (listen(this->fd, 20) != 0) {
		throw Socket::Exception(*this, "Listening on socket failed: " + std::string(std::strerror(errno)));
	}

	this->setReadFDStatus(FD_POLLING);

	l.log("Now listening on socket");
}

// Instead of actually reading from the file descriptor, we accept the connection
// and add it to the Client vector in the server
ssize_t Socket::doRead( void ) {
	sockaddr  address;
	socklen_t address_length;
	int client_socket = accept(this->getFileDescriptor(), (sockaddr *)&address, &address_length);

	if (client_socket < 0) {
		throw Socket::Exception(*this, "Accepting connection failed: " + std::string(std::strerror(errno)));
	}

	this->client_vector->emplace_back(client_socket, address, address_length, *this, this->l);

	return 1;
}

// Since we just want to keep listening for clients on this socket,
// we immediately mark the file descriptor as polling again
void Socket::readingDone( void ) {
	this->resetReadBuffer();
	this->setReadFDStatus(FD_POLLING);
}
