#if !defined(RESPONSE_HPP)
#define RESPONSE_HPP

#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/socket.h>
#include <unordered_map>
#include "Client.hpp"

enum class StatusCode
{
	Null = 0,
	OK = 200,
	Created = 201,
	Accepted = 202,
	NoContent = 204,
	Found = 302,
	NotModified = 304,
	BadRequest = 400,
	UnAuthorized = 401,
	Forbidden = 403,
	NotFound = 404,
	MethodNotAllowed = 405,
	RequestTimeout = 408,
	LenghtRequired = 411,
	PayloadToLarge = 413,
	URIToLong = 414,
	UnsupportedMediaType = 415,
	InternalServerError = 500,
	NotImplemented = 501,
	BadGateway = 502,
	ServiceUnavailable = 503,
	GatewayTimeout = 504
};
class Response
{
public:
	Response(Request &client_request);
	~Response();
	void	createResponse(void);
	void	addStatusLine(std::string &status_line);
	void	addHeader(std::string &header);
	void	addBody(std::string &body);
	void	sendResponse();

	void	Get_Response();
	void	Delete_Response();
	void	Post_Response();

private:

    std::string m_total_respnse;
	std::unordered_map<std::string, std::string> m_headers;
	StatusCode m_status;
	Request &m_client_request;
};

#endif // RESPONSE_HPP
