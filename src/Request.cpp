#include"Request.hpp"
#include"HelperFuncs.hpp"
#include"constants.hpp"

#define BUFFER_SIZE 1096

Request::Request() : m_content_length(0),  m_method(HTTPMethod::UNDEFINED), m_uri(""), m_keep_alive(false), m_loc(nullptr)
{
}

Request::~Request()
{
}

void Request::setMethod()
{
	size_t pos = m_total_request.find(' ');
	if (pos != std::string::npos)
	{
		std::string method = m_total_request.substr(0, pos);
		if (method == "GET")
			m_method = HTTPMethod::GET;
		else if(method == "POST")
			m_method = HTTPMethod::POST;
		else if(method == "DELETE")
			m_method = HTTPMethod::DELETE;
		else
			m_method = HTTPMethod::UNDEFINED;
	}
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

	this->extractPath();
	size_t start_headers = m_total_request.find(CRLF) + 2;
	size_t end_headers = m_total_request.find(CRLFCRLF);
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
			throw std::logic_error("invalid header format");
        i += 2; // Move past the "\r\n" delimiter
		j = i;
    }
	// for (const auto& pair : m_headers)
	// 	log(std::string(pair.first + " : " + pair.second), Color::Magenta);
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

ClientState	Request::readFromClient(int client_fd)
{
	size_t pos;
	char buffer[BUFFER_SIZE];

	m_bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0);
	if (m_bytes_read <= 0)
		return ClientState::ERROR;
	std::string str(buffer);
	str.resize(m_bytes_read);
	m_total_request += str;

	// log("\n====== incoming request  ======\n");
	// std::cout << m_total_request << std::endl;


	if (m_method == HTTPMethod::UNDEFINED)
		this->setMethod();

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
		this->parseHeaders();
		if (m_headers.find("Connection") != m_headers.end())
		{
			if (m_headers.at("Connection") == "keep-alive")
				m_keep_alive = true;
		}
		if (m_headers.find("Content-length") != m_headers.end())
		{
			m_content_length = std::stoi(m_headers.at("Content-length"));
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


void Request::handleLocation(Server *server)
{
	std::string raw_path = split(this->Get_URI(), "?")[0];
	std::string redir_path;
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
	// std::string root = m_loc->getRootPath();
	if (raw_path.find(m_loc->getRootPath()) == std::string::npos)
		m_final_path = joinPath({m_loc->getRootPath(), raw_path}, "/"); // still need to fix directory listing
	if (raw_path == "/")
		m_final_path = joinPath({m_final_path, m_loc->getIndices()[0]}, "/");
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

const std::string&	Request::Get_Body()
{
	return m_body;
}

const std::string& Request::Get_URI() 
{
	return m_uri;
}

const std::string& Request::Get_final_path() 
 {
	return m_final_path;
}

const std::string& Request::Get_redir_path() 
{
	return m_redirection_path;
}

const Location& Request::Get_location() 
{
	return *m_loc;
}

const HTTPMethod& 	Request::Get_Method() 
{
	return m_method;
}

const std::unordered_map<std::string, std::string>& Request::Get_Headers() 
{
	return m_headers;
}

const std::string& Request::Get_Request()
{
	return m_total_request;
}

const bool&	Request::Get_Keep_Alive()
{
	return m_keep_alive;
}