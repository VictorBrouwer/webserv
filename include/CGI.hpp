#pragma once

#include <iostream>
#include <string>
#include "Request.hpp"
#include "Response.hpp"
#include <vector>
#include <cstring>
#include "HelperFuncs.hpp"
#include <memory>
#include <sys/wait.h>

#define FORK_ERROR -1
#define PIPE_ERROR -1
#define READ 0
#define WRITE 1
#define CHILD 0

class CGI {
public:

    CGI(std::shared_ptr<Request> client_request);
    ~CGI();

    int     ExecuteScript(std::string path);

    int     read_fd  = -1;
    int     write_fd = -1;
    pid_t   pid = -1;

private:

    void    GetMethodParse();
    void    PostMethodParse();
    void    DeleteMethodParse();

    void    ParseEnviromentArray();
    void    ParseHeader(const std::string &header, const std::string &enviroment_name);
    void    GiveScriptDataSTDIN();

    char    **AllocateArgumentVector();
    char    **AllocateEnviroment();


    void    DeletePointerArray(char **arr, size_t index);



    char                        **m_envp;
    char                        **m_argv;
    std::vector<std::string>    m_enviroment_var;
    std::string                 m_path;
    std::shared_ptr<Request>    m_client_request;


};
