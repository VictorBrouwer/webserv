#include"Request.hpp"
#include"HelperFuncs.hpp"

Request::Request(/* args */)
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
	}
}

void Request::parseHeaders()
{
    std::string line;
    size_t i = 0;

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

void	Request::readFromClient(int client_fd)
{
	size_t BUFFER_SIZE = 1096;
	char buffer[BUFFER_SIZE];
	// std::string header_end;
	size_t pos;

	// m_bytes_read = read(client_fd, buffer, BUFFER_SIZE);
	m_bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0);
	m_total_request += std::string(buffer);
	this->setMethod();
	pos = m_total_request.find("\r\n\r\n");
	if (pos != std::string::npos)
	{
		this->parseHeaders();
		if (m_headers.find("Content-length") != m_headers.end())
		{
			m_content_length = std::stoi(m_headers.at("Content-length"));
			if (m_total_request.size() - (pos + 2) >= m_content_length)
				m_body = m_total_request.substr(pos + 2);
		}
	}
	log("\n====== incoming request  ======\n");
	std::cout << m_total_request << std::endl;
}