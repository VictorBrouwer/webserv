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
#include "PollableFileDescriptor.hpp"
#include <fcntl.h>
#include <map>
#include <memory>

enum class StatusCode
{
	Null = 0,
	OK = 200,
	Created = 201,
	Accepted = 202,
	NoContent = 204,
	MovedPermanently = 301,
	Found = 302,
	SeeOther = 303,
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

typedef int fd_t;
class Response : public ReadFileDescriptor, public WriteFileDescriptor
{
public:
	Response(std::shared_ptr<Request> client_request);
	Response(int status_code);
	~Response();

	void				addHeader();
	void				addHeader(int status_code);
	void				createResponse(Server *server);
	const std::string 	&getResponse() const;
	void				createRedirect();


private:

	bool			DoesFileExists();
	fd_t 			OpenFile(const std::string &path, int o_flag, int o_perm = 0644) noexcept(false);

	std::string		ExtensionExtractor(const std::string &path);
	void			ExecuteCGI();

	void			GetFile();
	void			DeleteFile();
	void			UploadFile();
	void 			respondWithDirectoryListing();
	std::string		customizeErrorPage(int status_code);

	void writingDone( void );
	void readingDone( void );

	void sendToClient( void );

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
			{204, 		     "No Content"},

			{301,	 "Moved Permanently"},
			{302,                "Found"},
			{303,			 "See Other"},
			{304,          "Not Modified"},

			{400,           "Bad Request"},
			{401,         "Unauthorized"},
			{403,            "Forbidden"},
			{404,             "Not Found"},
			{405,     "Method Not Allowed"},
			{408,       "Request Timeout"},
			{411,       "Length Required"},
			{413,      "Payload Too Large"},
			{414,           "URI Too Long"},
			{415, "Unsupported Media Type"},

			{500,  "Internal Server Error"},
			{501,       "Not Implemented"},
			{502,           "Bad Gateway"},
			{503,   "Service Unavailable"},
			{504,       "Gateway Timeout"}
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
