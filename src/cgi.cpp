#include "cgi.hpp"

/** 
 * REQUEST_METHOD: Specifies the HTTP request method, such as GET or POST.
 * QUERY_STRING: Contains the query string portion of the URL, if any.
 * CONTENT_LENGTH: Specifies the length of the content sent by the client for POST requests.
 * CONTENT_TYPE: Specifies the MIME type of the content sent by the client for POST requests.
 * HTTP_COOKIE: Contains any cookies sent by the client.
 * HTTP_USER_AGENT: Provides information about the client's browser or user agent.
 * REMOTE_ADDR: Contains the IP address of the client making the request.
 * REMOTE_HOST: Contains the hostname of the client making the request, if available.
 * SERVER_NAME: Contains the hostname of the server.
 * SERVER_PORT: Specifies the port number on which the server is listening for requests.
 * SERVER_PROTOCOL: Specifies the name and version of the protocol the client used to communicate with the server.
 * SERVER_SOFTWARE: Contains the name and version of the server software.
 * Constructors and Destructor
**/

cgi::cgi(Request &client_request) : m_client_request(client_request)
{
    switch (m_client_request.Get_Method())
	{
	case HTTPMethod::GET:
		this->GetMethodParse();
		break;
	default:
		std::cout << "DELETE / POST (W.I.P)" << std::endl;
		break;
	}
}

cgi::~cgi() {
    std::cout << "cgi Destructor called" << std::endl;
}

void    cgi::GetMethodParse()
{
    size_t pos;

    m_enviroment_var.push_back("REQUEST_METHOD=GET");

    pos = m_client_request.Get_Path().find('?');
    if (pos != std::string::npos)
        m_enviroment_var.push_back("QUERY_STRING=" + m_client_request.Get_Path().substr(pos));
    
    ParseHeader("Cookie", "HTTP_COOKIE");
    ParseHeader("User-Agent", "HTTP_USER_AGENT");
    // REMOTE_ADDR is not in the request
    ParseHeader("Host", "REMOTE_HOST");
    // SERVER 


}

void    cgi::ParseHeader(const std::string &header, const std::string &enviroment_name)
{
    std::unordered_map<std::string, std::string> DB_header(m_client_request.Get_Headers());
    std::unordered_map<std::string, std::string>::iterator it;

    it = DB_header.find(header);
    if (it != DB_header.end())
        m_enviroment_var.push_back(enviroment_name + "=" + it->second);
}


