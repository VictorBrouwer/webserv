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
#include <fstream>
#include <filesystem>
#include "Request.hpp"
#include "HelperFuncs.hpp"
#include <map>
#include <memory>

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
	Unauthorized = 401,
	Forbidden = 403,
	NotFound = 404,
	MethodNotAllowed = 405,
	RequestTimeout = 408,
	LengthRequired = 411,
	PayloadTooLarge = 413,
	URITooLong = 414,
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
	Response(std::shared_ptr<Request> client_request);
	~Response();

	void				addHeader();
	void				createResponse();
	const std::string 	&getResponse() const;
	void				clearResponse();

	void	Get_Response();
	void	Delete_Response();
	void	Post_Response();

private:

	bool			DoesFileExists();
	std::fstream 	OpenFile(std::ios_base::openmode) noexcept(false);
	void			ReadFile(std::fstream &file) noexcept(false);

	std::string		ExtensionExtractor(const std::string &path);

	std::string										m_body;
	StatusCode										m_status;
	std::shared_ptr<Request> 						m_client_request;
	std::unordered_map<std::string, std::string> 	m_headers;
	/* The Content DataBases for ease of lookup */
	std::unordered_map<int, std::string>			m_DB_status;
	std::unordered_map<std::string, std::string>	m_DB_ContentType;

    std::string 									m_total_response;
	unsigned int									m_content_length;


};

#endif // RESPONSE_HPP
