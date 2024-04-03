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

enum class HTTPMethod
{
	GET,
	POST,
	DELETE,
	UNDEFINED,
};

enum class HostPort
{
	HOST,
	PORT,
};

class Request
{
public:
	Request(/* args */);
	~Request();
	void			parseHeaders();
	void			extractPath();
	void			setMethod();
	ClientState		readFromClient(int client_fd);

	const std::string&	Get_Body();
	const std::string& 	Get_Path();
	const HTTPMethod& 	Get_Method();
	const std::string& 	Get_Request();
	std::string			extractHostPort(HostPort get);
	const bool& 			Get_Keep_Alive();
	const std::unordered_map<std::string, std::string>& Get_Headers();

private:
	size_t 		m_bytes_read;
	size_t 		m_content_length;
	// size_t 		m_content_bytes_read;
	HTTPMethod	m_method;
    std::string m_total_request;
    std::string m_path;
    std::unordered_map<std::string, std::string> m_headers;
    std::string m_body;
	bool		m_keep_alive;
};
