#if !defined(REQUEST_HPP)
#define REQUEST_HPP

#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <unordered_map>
#include <sys/socket.h>
#include "ClientState.hpp"

enum class HTTPMethod
{
	GET,
	POST,
	DELETE,
	UNDEFINED,
};

class Request
{
public:
	Request(/* args */);
	~Request();
	void			parseHeaders();
	std::string		extractPath();
	void			setMethod();
	ClientState		readFromClient(int client_fd);

	std::string	Get_Body();
	std::string Get_Path();
	HTTPMethod 	Get_Method();
	std::unordered_map<std::string, std::string> Get_Headers();

private:
	size_t 		m_bytes_read;
	size_t 		m_content_length;
	// size_t 		m_content_bytes_read;
	HTTPMethod	m_method;
    std::string m_total_request;
    std::string m_path;
    std::unordered_map<std::string, std::string> m_headers;
    std::string m_body;
};

#endif // REQUEST_HPP
