#pragma once

#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>

#include "Logger.hpp"
#include "PollableFileDescriptor.hpp"

class Socket : public ReadFileDescriptor {
	public:
		Socket(const std::string& interface, int port, const Logger& logger);
		~Socket();

		const std::string& getInterface( void ) const;
		int                getPort( void ) const;
		int	               getFileDescriptor( void ) const;

		std::string toString( void ) const;

		void startListening( void );

	private:
		int         port;
		std::string interface;

		sockaddr_in socket_struct;

		int         fd = -1; // Default to avoid close() calls on nonexistent fds

		Logger l;

	public:
		class Exception : public std::exception {
			private:
				const std::string _message;

			public:
				Exception(const Socket& socket, const std::string& reason) :
				_message("Exception for socket " + socket.toString() + " : " + reason) { }

				Exception(const std::string& reason) :
				_message("Socket exception : " + reason) { }

				virtual const char* what() const throw() {
					return _message.c_str();
				}
		};
};
