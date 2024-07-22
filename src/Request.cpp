#include"Request.hpp"
#include"HelperFuncs.hpp"
#include"constants.hpp"
#include <ranges>

Request::Request() : m_content_length(0),  m_method(HTTPMethod::UNDEFINED), m_uri(""), m_keep_alive(false), m_loc(nullptr), m_auto_index(false)
{
}

// This sets up the request based on the headers, then lets us continue
// reading for the body
Request::Request(std::string& request_headers, const Logger& logger, int socket_fd) {
	this->l.setLogLevel(logger.getLogLevel());
	this->socket_fd = socket_fd;

	std::size_t first_line_pos = request_headers.find("\r\n");
	std::string first_line = request_headers.substr(0, first_line_pos);

	if (std::ranges::count(first_line, ' ') != 2) {
		throw Request::Exception("Malformed first line");
	}

	l.log("Grabbing method, uri and protocol from first line");
	std::stringstream first_line_stream(std::move(first_line));
	std::string method, uri, protocol;
	first_line_stream >> method >> uri >> protocol;

	if (protocol != "HTTP/1.1") {
		throw Request::Exception("Unsupported protocol");
	}

	this->setMethod(method);
	this->m_uri = uri;

	this->m_total_request = request_headers;

	this->parseHeaders();
	this->setHostPortFromHeaders();
	// TODO find server/location based on headers, listen file descriptor and interface (or default server)

	// TODO set max_body_size based on server/location settings

	if (!this->getChunkedRequest()) {
		try {
			this->m_content_length = std::stoi(this->m_headers.at("Content-Length"));
		}
		catch(const std::exception& e) {
			this->m_content_length = 0;
		}
	}
}

Request::~Request()
{
}

void Request::setHostPortFromHeaders( void ) {
	try {
		std::string& host_header = this->m_headers.at("Host");
		size_t start_port_num = host_header.find(':');

		// Setting the host and port, if we are running as root the
		// default port is 80, else it's 8080
		if (start_port_num != std::string::npos) {
			this->host = host_header.substr(0, start_port_num);
			std::string substr = host_header.substr(start_port_num + 1);
			this->port = std::stoi(substr);
		}
		else {
			this->host = host_header;
			this->port = (geteuid() == 0 ? 80 : 8080);
		}
	}
	catch(const std::exception& e) {
		// Catching any exception here, from stoi not being able to
		// convert the port number to there being no Host header present
		throw Request::Exception("Could not set host/port from headers: " + std::string(e.what()));
	}
}

void Request::setMethod(const std::string& method)
{
	if (method == "GET")
		m_method = HTTPMethod::GET;
	else if(method == "POST")
		m_method = HTTPMethod::POST;
	else if(method == "DELETE")
		m_method = HTTPMethod::DELETE;
	else
		throw Request::Exception("unsupported HTTP method");
}

void Request::extractPath()
{
	size_t firstSpacePos = m_total_request.find(' ');
	if (firstSpacePos != std::string::npos)
	{
		size_t secondSpacePos = m_total_request.find(' ', firstSpacePos + 1);
		if (secondSpacePos != std::string::npos)
			m_uri = m_total_request.substr(firstSpacePos + 1, secondSpacePos - firstSpacePos - 1);
	}
}

void Request::parseHeaders()
{
	l.log("Parsing request headers");

	size_t start_headers = m_total_request.find(CRLF) + 2;
	size_t end_headers = m_total_request.find(CRLFCRLF);
	if (start_headers == std::string::npos || end_headers == std::string::npos)
		throw Request::Exception("Could not properly determine header boundaries");
	std::string headers_section = m_total_request.substr(start_headers, (end_headers - start_headers + 2));
	std::string line, key, value;
	size_t i = 0, j = 0;
	while ((i = headers_section.find("\r\n", i)) != std::string::npos)
	{
		line = headers_section.substr(j, (i-j));
		if (line.empty()) // Empty line indicates end of headers
			break;
		size_t pos = line.find(':');
		if (pos != std::string::npos)
		{
			key = line.substr(0, pos);
			value = line.substr(pos + 2);
			m_headers.emplace(key, value);
		}
		else
			throw Request::Exception("improperly formatted header");
        i += 2; // Move past the "\r\n" delimiter
		j = i;
	}
}

/**
 * @brief Select Which u want HOST or PORT inside get variable
*/
std::string	Request::extractHostPort(HostPort get)
{
	std::string ret;
	ret = this->m_headers.at("Host");
	size_t start_port_num = ret.find(':');
	switch (get)
	{
	case HostPort::HOST:
		if (start_port_num != std::string::npos)
			ret = ret.substr(0,start_port_num);
		break;
	case HostPort::PORT:
		if (start_port_num != std::string::npos)
			ret = ret.substr(start_port_num); // need to discuss what we want to do in the else case
		break;
	default:
		log("unsupported enum in exctractHostPort");
		break;
	}
	return ret;
}

void Request::setBody(const std::string& body) {
	this->m_body = body;
}

std::pair<std::string,int> Request::getHostPort() const {
	return {this->host, this->port};
}

const std::string& Request::getHost( void ) const {
	return this->host;
}

int Request::getPort( void ) const {
	return this->port;
}

ClientState	Request::readFromClient(int client_fd)
{
	size_t pos;
	char buffer[BUFFER_SIZE];

	m_bytes_read = read(client_fd, buffer, BUFFER_SIZE);
	if (m_bytes_read <= 0)
		return ClientState::ERROR;
	std::string str(buffer, m_bytes_read);
	m_total_request.append(str);

	// if (m_method == HTTPMethod::UNDEFINED)
		// this->setMethod();

	pos = m_total_request.find("\r\n\r\n");
	if (m_content_length != 0)
	{
		if (m_total_request.size() - (pos + 4) >= m_content_length)
		{
			m_body = m_total_request.substr(pos + 4);
			return ClientState::READING_DONE;
		}
		else
			return ClientState::LOADING;
	}
	if (pos != std::string::npos)
	{
		// std::cout << buffer << std::endl;
		this->parseHeaders();
		if (m_headers.find("Connection") != m_headers.end())
		{
			if (m_headers.at("Connection") == "keep-alive")
				m_keep_alive = true;
		}
		if (m_headers.find("Content-Length") != m_headers.end())
		{
			m_content_length = std::stoi(m_headers.at("Content-Length"));
			if (m_total_request.size() - (pos + 4) >= m_content_length)
			{
				m_body = m_total_request.substr(pos + 4);
				return ClientState::READING_DONE;
			}
			else
				return ClientState::LOADING;
		}
		else
			return ClientState::READING_DONE;
	}
	else
		return ClientState::LOADING;
}

/**
 * @brief this function takes the raw url, finds the matching location and applies the rules of that location
*/
void Request::handleLocation(Server *server) // still need to fix directory listing
{
	std::string raw_path = split(this->getURI(), "?", 1)[0];
	std::string redir_path;
	std::string temp_path;
	m_loc = server->findLocation(raw_path);
	if (m_loc->checkMethod(m_method) == false)
		throw std::runtime_error("invalid request method");
	if (m_loc->getReturnActive())
	{
		redir_path = m_loc->getReturnBody();
		if (redir_path != "" && redir_path[0] != '/')
			redir_path = "/" + redir_path;
		m_redirection_path = redir_path;
		return ;
	}
	if (raw_path.back() == '/' && m_method != HTTPMethod::POST)
	{
		if (m_loc->getAutoindexEnabled())
		{
			m_auto_index = true;
			raw_path = raw_path.substr(m_loc->getUri().length());
			m_final_path = joinPath({m_loc->getRootPath(), raw_path}, "/");;
			return;
		}
		if (raw_path.find(m_loc->getUri()) == 0)
			raw_path = raw_path.substr(m_loc->getUri().length());
		for (const auto &index : m_loc->getIndices())
		{
			temp_path = joinPath({m_loc->getRootPath(), raw_path, index}, "/");
			if (std::filesystem::exists(temp_path))
			{
				m_final_path = temp_path;
				return ;
			}
		}
		return; // forbidden
	}
	if (raw_path.find(m_loc->getUri()) == 0) // extract part after the location
		m_final_path = joinPath({m_loc->getRootPath(), raw_path.substr(m_loc->getUri().length())}, "/"); // add root path to the uri
	else
		m_final_path = joinPath({m_loc->getRootPath(), raw_path}, "/");
}

const std::string&	Request::getBody() const
{
	return m_body;
}

const std::string& Request::getURI() const
{
	return m_uri;
}

const std::string& Request::getFinalPath() const
 {
	return m_final_path;
}

const std::string& Request::getRedirPath() const
{
	return m_redirection_path;
}

const Location& Request::getLocation() const
{
	return *m_loc;
}

const HTTPMethod& 	Request::getMethod() const
{
	return m_method;
}

const std::unordered_map<std::string, std::string>& Request::getHeaders() const
{
	return m_headers;
}

const std::string& Request::getRequest() const
{
	return m_total_request;
}

const bool&	Request::getKeepAlive() const
{
	return m_keep_alive;
}

const bool&	Request::getAutoindex() const
{
	return m_auto_index;
}

size_t Request::getContentLength() const
{
	return m_content_length;
}

int Request::getSocketFD() const
{
	return socket_fd;
}

// std::size_t	Request::getMaxBodySize() const {
// 	try {
// 		return this->responding_server->getClientMaxBodySize();
// 	}
// 	catch(const std::exception& e) {
// 		return 0;
// 	}
// }

bool Request::getChunkedRequest() const {
	try {
		if (this->m_headers.at("Transfer-Encoding") == "chunked")
			return true;
	}
	catch(const std::out_of_range& e) { }

	return false;
}

bool Request::hasBody( void ) const {
	try {
		if (this->m_headers.at("Transfer-Encoding") == "chunked")
			return true;
	}
	catch(const std::out_of_range& e) { }

	try {
		if (std::stoi(this->m_headers.at("Content-Length")) > 0)
			return true;
	}
	catch(const std::out_of_range& e) { }

	return false;
}
