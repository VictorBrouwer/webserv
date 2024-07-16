#pragma once

#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <fstream>
#include <poll.h>
#include <unordered_map>
#include <vector>
#include <memory>

#include "Poll.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Socket.hpp"
#include "HelperFuncs.hpp"
#include "Configuration.hpp"
#include "Logger.hpp"
#include "ConfigShared.hpp"

class HTTPServer : public ConfigShared {
	public:
		HTTPServer(Configuration &config, const Logger& l);

		~HTTPServer();

		std::vector<Server>::iterator getServerMutableIterator( void );
		std::vector<Server>::iterator getServerMutableEnd( void );
		std::vector<Server>& 		  getServerVector( void );

		std::vector<Socket>::iterator getSocketIterator( void );
		std::vector<Socket>::iterator getSocketEnd( void );

		std::vector<Client>::iterator getClientIterator( void );
		std::vector<Client>::iterator getClientEnd( void );


		void startListening( void );

		void doPollLoop( void );

		inline static std::unique_ptr<HTTPServer> instance;

	private:
		Logger l;

		// A Socket is a file descriptor on which we listen for new clients.
		// It is a separate entity from servers because multiple servers
		// may listen on the same socket, or a different subset of them.
		std::vector<Socket> sockets;

		// A Server is a virtual server registered in the config that listens
		// on at least one of our Sockets and describes how it wants to answer
		// requests. When a Request comes in, it's matched to a Server, which
		// constructs a Response for it.
		std::vector<Server> servers;

		// A Client is a currently open connection on which we might receive
		// Requests, which we need to answer with a Response.
		//
		// Each Client handles one Request at a time, meaning its response
		// will be built inside the Client class.
		std::vector<Client> clients;

		void setupSockets( void );

		// Poll stuff

		std::vector<pollfd> poll_vector;
		std::unordered_map<int,ReadFileDescriptor*>  read_fd_pointers;
		std::unordered_map<int,WriteFileDescriptor*> write_fd_pointers;

		void assemblePollQueue( void );
		void runPoll( void );
		void handleEvents( void );
		void cleanUpPoll( void );

		void addReadFileDescriptorToPoll(ReadFileDescriptor* read_fd);
		void addWriteFileDescriptorToPoll(WriteFileDescriptor* read_fd);

	public:
		class Exception : public std::exception {
			private:
				const std::string _message;

			public:
				Exception(const std::string& reason) :
				_message("HTTPServer Exception : " + reason) { }

				virtual const char* what() const throw() {
					return _message.c_str();
				}
		};
};
