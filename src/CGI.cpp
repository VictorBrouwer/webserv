#include "CGI.hpp"

/**
 * Enviroment VARS
 * REQUEST_METHOD: Specifies the HTTP request method, such as GET or POST.
 * QUERY_STRING: Contains the query string portion of the URL, if any.
 * CONTENT_LENGTH: Specifies the length of the content sent by the client for POST requests. (POST)
 * CONTENT_TYPE: Specifies the MIME type of the content sent by the client for POST requests.(POST)
 * HTTP_COOKIE: Contains any cookies sent by the client.
 * HTTP_USER_AGENT: Provides information about the client's browser or user agent.
 * REMOTE_HOST: Contains the hostname of the client making the request, if available.
 * SERVER_NAME: Contains the hostname of the server.
 * SERVER_PORT: Specifies the port number on which the server is listening for requests.
 * SERVER_PROTOCOL: Specifies the name and version of the protocol the client used to communicate with the server.
 * SERVER_SOFTWARE: Contains the name and version of the server software.
 * Constructors and Destructor
**/

/**
 * @brief Construct a new CGI::CGI object
 *
 * @param client_request
 */
CGI::CGI(std::shared_ptr<Request> client_request) : m_client_request(client_request)
{
    m_path = m_client_request->getFinalPath();

    log(m_path, L_Debug);

    switch (m_client_request->getMethod())
	{
	case HTTPMethod::GET:
		this->GetMethodParse();
		break;
    case HTTPMethod::POST:
        this->PostMethodParse();
		break;
    case HTTPMethod::DELETE:
        this->DeleteMethodParse();
		break;
	default:
		log("Unsopported Method Passed! CGI", L_Error);
		break;
	}

}

CGI::~CGI() {
    // std::cout << "CGI Destructor called" << std::endl;
    log("CGI Destructor called");
}


/**
 * @brief Creating the enviroment variable list for the GET method.
 *
 */
void    CGI::GetMethodParse()
{
    m_enviroment_var.push_back("REQUEST_METHOD=GET");
    ParseEnviromentArray();
}
/**
 * @brief Creating the enviroment variable list for the POST method.
 *
 */
void    CGI::PostMethodParse()
{
    m_enviroment_var.push_back("REQUEST_METHOD=POST");
    ParseHeader("Content-length", "CONTENT_LENGTH");
    ParseHeader("Content-type", "CONTENT_TYPE");
    ParseEnviromentArray();
}
/**
 * @brief Creating the enviroment variable list for the DELETE method.
 *
 */
void    CGI::DeleteMethodParse()
{
    m_enviroment_var.push_back("REQUEST_METHOD=DELETE");
    ParseHeader("Content-length", "CONTENT_LENGTH");
    ParseHeader("Content-type", "CONTENT_TYPE");
    ParseEnviromentArray();
}

void    CGI::ParseEnviromentArray()
{
    size_t pos;
    std::string str;
    pos = m_path.find('?');
    if (pos != std::string::npos)
        m_enviroment_var.push_back("QUERY_STRING=" + m_path.substr(pos));

    ParseHeader("Cookie", "HTTP_COOKIE");
    ParseHeader("User-Agent", "HTTP_USER_AGENT");
    ParseHeader("Host", "REMOTE_HOST");
    m_enviroment_var.push_back("SERVER_NAME=" + m_client_request->extractHostPort(HostPort::HOST));
    m_enviroment_var.push_back("SERVER_PORT=" + m_client_request->extractHostPort(HostPort::PORT));
    pos = m_client_request->getRequest().find("HTTP");
    m_enviroment_var.push_back("SERVER_PROTOCOL=" + m_client_request->getRequest().substr(pos, m_client_request->getRequest().find("\r\n") - pos));
    m_enviroment_var.push_back("SERVER_SOFTWARE=Webserv/1.0.0 (Unix) Python/3.10.12");
}

/**
 * @brief This function is responsible for executing the script and return a file disciptor to read the output of the scripts.
 *
 * @warning This function can throw std::logic_error && std::bad_alloc
 *
 * @throw std::logic_error std::bad_alloc
 * @return int file descriptor
 */
int CGI::ExecuteScript(std::string path) noexcept(false)
{
    pid_t pid;
    size_t pos;
    int pipefds[2];
    int writepipefds[2];

    pos = path.find('?');
    if (pos != std::string::npos)
        path.erase(pos);

    if (access(path.c_str(), X_OK) == 0)
        throw StatusCode::Forbidden;

    m_envp = this->AllocateEnviroment();
    m_argv = this->AllocateArgumentVector();

    if (pipe(pipefds) == PIPE_ERROR || pipe(writepipefds) == PIPE_ERROR)
    {
        this->DeletePointerArray(this->m_envp, this->m_enviroment_var.size());
        throw StatusCode::InternalServerError;
    }

    pid = fork();
    if (pid == FORK_ERROR)
    {
        close(pipefds[READ]);
        close(pipefds[WRITE]);
        close(writepipefds[READ]);
        close(writepipefds[WRITE]);
        this->DeletePointerArray(this->m_envp, this->m_enviroment_var.size());
        throw StatusCode::InternalServerError;
    }
    if (pid == CHILD)
    {
        log("Child is executing script", L_Info);
        // if (m_client_request->getMethod() == HTTPMethod::POST)
        //     this->GiveScriptDataSTDIN();
        close(pipefds[READ]);
        dup2(pipefds[WRITE], STDOUT_FILENO);
        close(pipefds[WRITE]);
        close(writepipefds[WRITE]);
        dup2(writepipefds[READ], STDIN_FILENO);
        close(writepipefds[READ]);
        execve("/usr/bin/python3", m_argv, m_envp); // This needs to be changed if we are going to support multiple code langs.
        exit(1);
    }
    // if (this->m_client_request->getMethod() != HTTPMethod::POST) {
    //     close(pipefds[WRITE]);
    // }
    this->DeletePointerArray(this->m_envp, this->m_enviroment_var.size());
    this->DeletePointerArray(this->m_argv, 2);

    close(pipefds[WRITE]);
    close(writepipefds[READ]);

    this->read_fd = pipefds[READ];
    this->write_fd = writepipefds[WRITE];

    this->pid = pid;

    return (pipefds[READ]);
}

/**
 * @brief Parses the header and adds it to the enviroment variable.
 *
 * @param header the header to be parsed
 * @param enviroment_name
 */
void    CGI::ParseHeader(const std::string &header, const std::string &enviroment_name)
{
    std::unordered_map<std::string, std::string> DB_header(m_client_request->getHeaders());
    std::unordered_map<std::string, std::string>::iterator it;

    it = DB_header.find(header);
    if (it != DB_header.end())
        m_enviroment_var.push_back(enviroment_name + "=" + it->second);
}

/**
 * @brief Allocates the enviroment variable for the script.
 *
 * @warning This function can throw std::bad_alloc
 *
 * @return enviroment variable
 */
char    **CGI::AllocateEnviroment() noexcept(false)
{
    char **enviroment = NULL;
    size_t index = 0;

    try
    {
        enviroment = new char*[m_enviroment_var.size() + 1]; // + 1 for NULL
        for (auto it = m_enviroment_var.begin(); it != m_enviroment_var.end(); it++)
        {
            enviroment[index] = new char[it->size() + 1];
            strcpy(enviroment[index], it->c_str());
            index++;
        }
        enviroment[index] = NULL;
    }
    catch(const std::exception& e)
    {
        if (enviroment != NULL)
            this->DeletePointerArray(enviroment, index);
        throw StatusCode::InternalServerError; // rethrows original exception bad_alloc
    }
    return (enviroment);
}

/**
 * @brief Allocoates the argument vector for the script.
 *
 * @warning This function can throw std::bad_alloc
 *
 * @return Argument vector
 */
char    **CGI::AllocateArgumentVector() noexcept(false)
{
    char **argv = NULL;
    size_t index = 0;

    try
    {
        argv = new char*[3];

        argv[index] = new char[sizeof("/usr/bin/python3") + 1]; // +1 for NULL
        strcpy(argv[0], "/usr/bin/python3");
        index++;
        log(m_path);
        argv[index] = new char[m_path.size() + 1];
        strcpy(argv[1], m_path.c_str());
        index++;

        argv[index] = NULL;
    }
    catch(const std::exception& e)
    {
        if (argv != NULL)
            this->DeletePointerArray(argv, index);
        throw StatusCode::InternalServerError;
    }
    return (argv);
}

void    CGI::DeletePointerArray(char **arr, size_t index)
{
    int i = index;

    while (i >= 0)
    {
        delete[] arr[i];
        i--;
    }
    delete[] arr;
}

void    CGI::GiveScriptDataSTDIN()
{
    int data_pipe[2];

    if (pipe(data_pipe) == PIPE_ERROR)
    {
        this->DeletePointerArray(this->m_envp, this->m_enviroment_var.size());
        exit(1);
    }
    write(data_pipe[WRITE], m_client_request->getBody().c_str(), m_client_request->getBody().size()); //Warning!! this has an write operation!
    dup2(data_pipe[WRITE], STDIN_FILENO);
    close(data_pipe[WRITE]);
    close(data_pipe[READ]);
}
