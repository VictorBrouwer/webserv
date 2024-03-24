#pragma once

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
#include "Server.hpp"
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
	void				createResponse(Server *server);
	const std::string 	&getResponse() const;
	void				clearResponse();
	std::string			parsePath();

	void	ParseResponse(std::ios_base::openmode mode);

private:

	bool			DoesFileExists();
	std::fstream 	OpenFile(std::ios_base::openmode) noexcept(false);
	void			ReadFile(std::fstream &file) noexcept(false);

	std::string		ExtensionExtractor(const std::string &path);
	void			ExecuteCGI();

	Server				 							*m_server;
	HTTPMethod										m_method;
	std::string										m_body;
	std::string										m_path;
	StatusCode										m_status;
	bool											m_CGI;
	std::shared_ptr<Request> 						m_client_request;
	std::unordered_map<std::string, std::string> 	m_headers;

    std::string 									m_total_response;


	// These are the supported Exit codes. 
	// The response will look at the m_status to determine its response exit message.
	static const inline std::unordered_map<int, std::string>			m_DB_status = {
			{000,                 "Null"},

			{200,                   "OK"},
			{201,              "Created"},
			{202,             "Accepted"},
			{204, 		     "NoContent"},

			{302,                "Found"},
			{304,          "NotModified"},

			{400,           "BadRequest"},
			{401,         "Unauthorized"},
			{403,            "Forbidden"},
			{404,             "NotFound"},
			{405,     "MethodNotAllowed"},
			{408,       "RequestTimeout"},
			{411,       "LengthRequired"},
			{413,      "PayloadTooLarge"},
			{414,           "URITooLong"},
			{415, "UnsupportedMediaType"},

			{500,  "InternalServerError"},
			{501,       "NotImplemented"},
			{502,           "BadGateway"},
			{503,   "ServiceUnavailable"},
			{504,       "GatewayTimeout"}
		};


	// These are the supported content-Types if you want to add any more supported content types you can add them here
	// Format: [file extension][Content-Type]
	static const inline std::unordered_map<std::string, std::string>	m_DB_ContentType = {
			{"html",      "text/html"},
			{"txt",      "text/plain"},

			{"xml", "application/xml"},
			{"x-www-form-urlencoded", "application/x-www-form-urlencoded"},

			{"jpeg",     "image/jpeg"},
			{"jpg",       "image/jpg"},
			{"png", 	  "image/png"},
			{"gif", 	  "image/gif"},
			{"ico",    "image/x-icon"},

			{"mpeg", 	 "audio/mpeg"},
			{"ogg", 	  "audio/ogg"},

			{"mp4", 	  "video/mp4"},
			{"webm", 	 "video/webm"},

			{"form-data", "multipart/form-data"},
		};

};
