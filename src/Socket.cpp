#include "Socket.hpp"
#include <cerrno>
#include <cstring>

Socket::Socket(const std::string& interface, int port, const Logger& logger) : l(logger) {
	this->interface = interface;
	this->port      = port;

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
	const int reuse = 1;
	if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) != 0) {
		throw Socket::Exception(*this, "Could not configure socket options: " + std::string(std::strerror(errno)));
	}

	if (bind(this->fd, (sockaddr *)&(this->socket_struct), sizeof(this->socket_struct)) != 0) {
		throw Socket::Exception(*this, "Could not bind socket to address: " + std::string(std::strerror(errno)));
	}
}

Socket::~Socket() {
	if (this->fd > 0) {
		close(this->fd);
	}
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

void Socket::startListening( void ) const {
	if (listen(this->fd, 20) != 0) {
		throw Socket::Exception(*this, "Listening on socket failed: " + std::string(std::strerror(errno)));
	}

	l.log("Now listening on socket");
}
