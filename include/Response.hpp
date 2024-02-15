#if !defined(RESPONSE_HPP)
#define RESPONSE_HPP

#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/socket.h>
#include "ClientState.hpp"

class Response
{
public:
	Response(/* args */);
	~Response();
	void	addStatusLine(std::string &status_line);
	void	addHeader(std::string &header);
	void	addBody(std::string &body);
	void	sendResponse();
private:
    std::string m_total_respnse;
	std::string m_path;
};

#endif // RESPONSE_HPP
