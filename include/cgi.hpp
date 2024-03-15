#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <string>
#include <Request.hpp>
#include <vector>
#include <cstring>
#include "HelperFuncs.hpp"

#define FORK_ERROR -1
#define PIPE_ERROR -1
#define READ 0
#define WRITE 1
#define CHILD 0

class cgi {
public:

    cgi(Request &client_request);
    ~cgi();
    int     ExecuteScript(std::string path);

private:

    void    GetMethodParse();
    void    ParseHeader(const std::string &header, const std::string &enviroment_name);

    char    **AllocateArgumentVector();
    char    **AllocateEnviroment();


    void    DeletePointerArray(char **arr, size_t index);


    
    char                        **m_envp;
    char                        **m_argv;
    std::vector<std::string>    m_enviroment_var;
    std::string                 m_path;
    Request                     &m_client_request;


};

#endif
