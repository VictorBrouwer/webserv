#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <string>
#include <Request.hpp>
#include <vector>

class cgi {
public:

    cgi(Request &client_request);
    ~cgi();

    // Copy constructor
    cgi(const cgi &obj);
    // Operator overload
    cgi &operator=(const cgi &obj);

private:

    void    GetMethodParse();
    void    ParseHeader(const std::string &header, const std::string &enviroment_name);


    char                        **m_envp;
    std::vector<std::string>    m_enviroment_var;
    Request                     &m_client_request;


};

#endif
