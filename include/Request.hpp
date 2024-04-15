#pragma once

#define BUFFER_SIZE 10906

#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <unordered_map>
#include <sys/socket.h>
#include "ClientState.hpp"
#include "Location.hpp"
#include <filesystem>
#include "Server.hpp"

enum class HostPort
{
	HOST,
	PORT,
};

class Request {
	public:
		Request(/* args */);
		Request(std::string& request_headers, const Logger& logger, int socket_fd);

		~Request();

		void		parseHeaders();
		void		extractPath();
		void		setHostPortFromHeaders( void );
		void		setMethod(const std::string& method);
		ClientState	readFromClient(int client_fd);

		const std::string&	getBody() const;
		const std::string& 	getURI() const;
		const std::string&	getFinalPath() const;
		const std::string&	getRedirPath() const;
		const Location&		getLocation() const;
		const HTTPMethod& 	getMethod() const;
		const std::string& 	getRequest() const;
		const bool& 	 	getAutoindex() const;
		size_t				getContentLength() const;
		const bool& 		getKeepAlive() const;

		std::string			extractHostPort(HostPort get);
		std::pair<std::string,int> getHostPort() const;
		const std::string&	getHost( void ) const;
		int                 getPort( void ) const;
		const std::unordered_map<std::string, std::string>& getHeaders() const;
		void				handleLocation(Server *server);

	private:
		Logger		l = Logger("Request");

		size_t 		m_bytes_read = 0;
		size_t 		m_content_length = 0;
		HTTPMethod	m_method = HTTPMethod::UNDEFINED;
		std::string m_total_request;
		std::string m_uri;
		std::unordered_map<std::string, std::string> m_headers;
		std::string m_body;
		bool		m_keep_alive = false;
		std::string m_redirection_path;
		std::string m_final_path;
		Location*	m_loc = nullptr;
		bool		m_auto_index = false;

		int         port;
		std::string host;
		// The listen sockets we received this request on
		int			socket_fd = -1;

	public:
		class Exception : public std::exception {
			private:
				const std::string message;

			public:
				Exception(const std::string& message) : message("Invalid request: " + message) { }

				virtual const char* what() const throw() {
					return message.c_str();
				}
		};
};
