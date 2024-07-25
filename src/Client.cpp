#include <chrono>
#include "Client.hpp"
#include "Socket.hpp"
#include "HTTPServer.hpp"
#include "HelperFuncs.hpp"

Client::Client(int fd, sockaddr address, socklen_t addr_len, const Socket &socket, const Logger &logger) : ReadFileDescriptor(fd), WriteFileDescriptor(fd),
																										   l(logger), socket(socket)
{
	this->l.setDefaultContext("Client");
	this->fd = fd;

	this->address = address;
	this->address_length = addr_len;

	this->read_timeout_seconds = 20;
	this->write_timeout_seconds = 20;

	l.log("Marking client socket " + std::to_string(this->fd) + " as ready for reading", L_Info);
	this->setReadFDStatus(FD_POLLING);
	this->read_start_time = std::chrono::steady_clock::now();
}

Client::Client(const Client &src) : ReadFileDescriptor(src), WriteFileDescriptor(src), socket(src.socket)
{
	*this = src;
}

Client &Client::operator=(Client &src)
{
	this->fd = src.fd;

	this->l = src.l;
	this->address = src.address;
	this->address_length = src.address_length;

	return *this;
}

Client &Client::operator=(const Client &src)
{
	this->fd = src.fd;

	this->l = src.l;
	this->address = src.address;
	this->address_length = src.address_length;

	return *this;
}

Client::Client(Client &&to_move) : socket(to_move.socket)
{
	this->fd = to_move.fd;
	to_move.fd = -1;

	this->l = to_move.l;
	this->address = to_move.address;
	this->address_length = to_move.address_length;
}

Client::~Client()
{
}

// Callback for reading
void Client::afterRead(void)
{
	std::string stream_contents = std::move(this->read_buffer).str();

	if (this->reading_body)
	{
		this->afterReadDuringBody(stream_contents);
	}
	else
	{
		this->afterReadDuringHeaders(stream_contents);
	}

	this->read_buffer.str(std::move(stream_contents));
}

void Client::afterReadDuringHeaders(std::string &stream_contents)
{
	try {
		std::size_t header_boundary = stream_contents.find("\r\n\r\n");

		if (header_boundary != std::string::npos)
		{
			std::string headers = stream_contents.substr(0, header_boundary + 4);

			// Put the rest back into the stringstream and reset the bytes_read
			stream_contents.erase(0, header_boundary + 4);
			this->bytes_read = stream_contents.size();

			// We are now going to read the body, so we want to delimit based on
			// chunks or on content-size
			l.log("Finished reading headers, constructing request.");

			this->m_request.reset(new Request(headers, l, this->socket.getFileDescriptor()));
			this->extractServer();

			if (this->m_request->hasBody())
			{
				l.log("Request has body, checking if we have everything already.");

				if (this->m_request->getChunkedRequest())
				{
					l.log("Chunked request");
				}
				else
				{
					if (this->m_request->getContentLength() <= this->bytes_read)
					{
						l.log("Full body received, passing it on.");
						this->m_request->setBody(this->read_buffer.str());
						this->m_request->m_total_request.append(this->read_buffer.str());
						this->setReadFDStatus(FD_DONE);
					}
					else
					{
						l.log("Full body is not in yet, continuing to read.");
						this->reading_body = true;
					}
				}

				this->chunked_request = m_request->getChunkedRequest();
				if (!chunked_request)
				{
					this->body_limit = m_server->getClientMaxBodySize();
				}
			}
			else
			{
				l.log("No body, done reading this request.");
				this->setReadFDStatus(FD_DONE);
			}
		}
		else if (this->bytes_read > header_limit)
		{
			l.log("Max header size exceeded, cutting off the connection", L_Warning);

			// Set up error Response and hit send
			throw(413);
		}
	}
	catch (int status_code) {
		this->m_response.reset(new Response(status_code));
		this->m_response->sendToClient();
		if (this->getReadFDStatus() == FD_POLLING)
			this->setReadFDStatus(FD_ERROR);
	}
	catch (std::exception& e) {
		this->m_response.reset(new Response(500));
		this->m_response->sendToClient();
		if (this->getReadFDStatus() == FD_POLLING)
			this->setReadFDStatus(FD_ERROR);
	}
}

void Client::afterReadDuringBody(std::string &stream_contents)
{
	try {
		if (this->bytes_read > body_limit)
		{
			l.log("Max body size exceeded, cutting off the connection.", L_Warning);
			this->setReadFDStatus(FD_ERROR);

			// Set up error Response and hit send
			throw(413);
		}
		else
		{
			if (this->chunked_request && stream_contents.find("\r\n0\r\n\r\n") != std::string::npos)
			{
				l.log("Found zero chunk, done reading.");
				this->setReadFDStatus(FD_DONE);
			}
			else if (this->bytes_read >= this->m_request->getContentLength())
			{
				l.log("Read all of Content-Length, done reading.");
				this->setReadFDStatus(FD_DONE);
			}
			else
			{
				l.log("Read " + std::to_string(this->bytes_read) + " bytes, continuing.");
			}

			if (this->getReadFDStatus() == FD_DONE)
			{
				this->m_request->setBody(stream_contents);
				this->m_request->m_total_request.append(stream_contents);
			}
		}
	}
	catch (int status_code) {
		this->m_response.reset(new Response(status_code));
		this->m_response->sendToClient();
		if (this->getReadFDStatus() == FD_POLLING)
			this->setReadFDStatus(FD_ERROR);
	}
	catch (std::exception& e) {
		this->m_response.reset(new Response(500));
		this->m_response->sendToClient();
		if (this->getReadFDStatus() == FD_POLLING)
			this->setReadFDStatus(FD_ERROR);
	}
}

// If we have to keepalive, keep the file descriptor open
void Client::readingDone(void)
{
	if (this->getReadFDStatus() != FD_DONE) {
		l.log("Client timed out, sending canned timeout response.", L_Warning);
		m_response.reset(new Response(408));
		m_response->sendToClient();
	}
	else {
		l.log("Full request received!");
		l.log(this->m_request->getRequest());

		// Prepare the response
		try {
			// If this goes wrong, it's an invalid request
			this->checkRequestSyntax(m_request->getRequest());
		}
		catch (const std::exception &e) {
			m_response.reset(new Response(400));
			m_response->sendToClient();
			return;
		}

		try {
			// If this goes wrong, the method is not allowed for this location
			m_request->handleLocation(m_server);
		}
		catch (const std::exception &e) {
			m_response.reset(new Response(401));
			m_response->sendToClient();
			return;
		}

		try
		{
			// If this goes wrong, it's an error on our end
			m_response.reset(new Response(this->m_request));
			m_response->createResponse(m_server);
		}
		catch (int error_code)
		{
			l.log("Serving canned error response.", L_Info);
			m_response.reset(new Response(error_code));
			m_response->sendToClient();
		}
		catch (const std::exception &e)
		{
			l.log("Serving canned error response.", L_Info);
			m_response.reset(new Response(500));
			m_response->sendToClient();
		}
	}
}

void Client::writingDone(void)
{
	m_request.reset(new Request);
	m_response.reset(new Response(m_request));

	if (this->m_request->getKeepAlive())
	{
		l.log("Connection should be kept alive, resetting.");
		this->setReadFDStatus(FD_POLLING);
		this->read_start_time = std::chrono::steady_clock::now();
	}
	else
	{
		l.log("Done writing, connection should be closed.");
		close(this->fd);
		this->setWriteFDStatus(FD_HUNG_UP);
	}
}

std::shared_ptr<Request> &Client::getRequest(void)
{
	return this->m_request;
}

std::shared_ptr<Response> &Client::getResponse(void)
{
	return this->m_response;
}

void Client::addToWriteBuffer(const std::string &str)
{
	this->write_buffer << str;
}

// Legacy

ClientState &Client::getState()
{
	return this->m_state;
}

void Client::checkRequestSyntax(const std::string &request)
{
	std::istringstream iss(request);
	std::vector<std::string> lines;
	std::string line;
	while (std::getline(iss, line))
		lines.push_back(line);
	// Check the minimum number of lines
	if (lines.size() < 3)
		throw std::runtime_error("invalid HTTP-request");
	const std::string &request_line = lines[0];
	std::istringstream request_line_stream(request_line);
	std::string method, path, http_version;
	request_line_stream >> method >> path >> http_version;
	if (path[0] != '/')
		throw std::runtime_error("invalid HTTP-request");
	if (http_version != "HTTP/1.1")
		throw std::runtime_error("invalid HTTP-request");
}

void Client::extractServer()
{
	int socket_fd = this->m_request->getSocketFD();
	for (auto &server : HTTPServer::instance->getServerVector())
	{
		for (const auto &servername : server.getServerNames())
		{
			if (servername == this->m_request->getHost() && server.containsSocket(socket_fd))
			{
				m_server = &server;
				l.log("Found server.");
				return;
			}
		}
	}
	if (!m_server)
	{
		for (auto &server : HTTPServer::instance->getServerVector())
		{
			if (server.containsSocket(socket_fd))
			{
				m_server = &server;
				l.log("Found server.");
				return;
			}
		}
	}
}
