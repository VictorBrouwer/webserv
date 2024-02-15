#include"Request.hpp"
#include"HelperFuncs.hpp"

Request::Request() : m_content_length(0),  m_method(HTTPMethod::UNDEFINED), m_path("")
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

std::string Request::extractPath()
{
    size_t firstSpacePos = m_total_request.find(' ');
    if (firstSpacePos != std::string::npos)
	{
        size_t secondSpacePos = m_total_request.find(' ', firstSpacePos + 1);
        if (secondSpacePos != std::string::npos)
            return m_total_request.substr(firstSpacePos + 1, secondSpacePos - firstSpacePos - 1);
    }
    return "";
}

void Request::parseHeaders()
{
    std::string line;
    size_t i = 0;

	if (m_path == "")
		m_path = this->extractPath();
    while ((i = m_total_request.find("\r\n", i)) != std::string::npos) {
        line = m_total_request.substr(0, i);
        if (line.empty()) // Empty line indicates end of headers
            break;
        size_t pos = line.find(':');
        if (pos != std::string::npos)
		{
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 2);
            m_headers.emplace(key, value);
        }
        i += 2; // Move past the "\r\n" delimiter
    }
}

ClientState	Request::readFromClient(int client_fd)
{
	size_t BUFFER_SIZE = 1096;
	char buffer[BUFFER_SIZE];
	size_t pos;

	m_bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0);
	m_total_request += std::string(buffer);
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


std::string	Request::Get_Body() 
{
	return m_body;
}
std::string Request::Get_Path() 
{
	return m_path;
}
HTTPMethod 	Request::Get_Method() 
{
	return m_method;
}
std::unordered_map<std::string, std::string> Request::Get_Headers() 
{
	return m_headers;
}