#if !defined(REQUEST_HPP)
#define REQUEST_HPP

#include <string>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <unordered_map>

class Request
{
public:
	Request(/* args */);
	~Request();
private:
	std::string m_method;
    std::string m_path;
    std::unordered_map<std::string, std::string> m_headers;
    std::string m_body;
};





#endif // REQUEST_HPP
