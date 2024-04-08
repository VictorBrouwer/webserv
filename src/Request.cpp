#include"Request.hpp"
#include"HelperFuncs.hpp"
#include"constants.hpp"

Request::Request() : m_content_length(0),  m_method(HTTPMethod::UNDEFINED), m_uri(""), m_keep_alive(false), m_loc(nullptr), m_auto_index(false)
{
}

// Grab the full request out of the stringstream
// Validate the first line: protocol, method, path, bad request if failure
// Parse the headers into the m_headers
// Find a server for the given host and socket, default server if we can't find one
// Check whether we are within max_body_size for the given server, payload too large if failure
// Parse the body:
//   - If content-length is not given, the entire request is the body
//   - If content-length is given and we have less data, malformed request
//   - If content-length is given and we have more data, discard the rest or malformed request
//   - If transfer-encoding is chunked, unchunk it
// We should now be ready to construct a response

Request::Request(std::stringstream& request_data) {
	this->m_total_request = request_data.str(); // Reallocation is not nice but we need the stream for the method/uri/protocol
	std::string method, uri, protocol;
	request_data >> method >> uri >> protocol;
	if (protocol != "HTTP/1.1") {
		throw Request::Exception("Unsupported protocol");
	}
	this->setMethod(method);
	this->m_uri = uri;

	this->parseHeaders();

}

Request::~Request()
{
}

void Request::splitBody( void ) {

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
	size_t start_headers = m_total_request.find(CRLF) + 2;
	size_t end_headers = m_total_request.find(CRLFCRLF);
	if (start_headers == std::string::npos || end_headers == std::string::npos)
		throw Request::Exception("Could not properly determine header boundaries");
	std::string headers_section = m_total_request.substr(start_headers, (end_headers-start_headers));
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
		std::cout << buffer << std::endl;
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

std::string Request::joinPath(std::vector<std::string> paths, std::string delimeter)
{
    std::string joined_path;

    for (size_t i = 0; i < paths.size(); i++)
    {
        std::string stripped = strip(paths[i], "/");
        if (stripped != "")
            joined_path += stripped + delimeter;
    }

    if (paths.back() == "/" || paths.back().back() != '/')
    {
        joined_path.pop_back();
    }
    return joined_path;
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
