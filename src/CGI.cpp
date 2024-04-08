#include "CGI.hpp"

/**
 * Enviroment VARS
 * REQUEST_METHOD: Specifies the HTTP request method, such as GET or POST.
 * QUERY_STRING: Contains the query string portion of the URL, if any.
 * CONTENT_LENGTH: Specifies the length of the content sent by the client for POST requests. (POST)
 * CONTENT_TYPE: Specifies the MIME type of the content sent by the client for POST requests.(POST)
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

/**
 * @brief Construct a new CGI::CGI object
 *
 * @param client_request
 */
CGI::CGI(std::shared_ptr<Request> client_request) : m_client_request(client_request)
{
    m_path = m_client_request->getURI();

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
    std::cout << "CGI Destructor called" << std::endl;
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
    // REMOTE_ADDR is not in the request
    ParseHeader("Host", "REMOTE_HOST");
    m_enviroment_var.push_back("SERVER_NAME=" + m_client_request->extractHostPort(HostPort::HOST));
    m_enviroment_var.push_back("SERVER_PORT=" + m_client_request->extractHostPort(HostPort::PORT));

    pos = m_client_request->getRequest().find("HTTP");
	m_enviroment_var.push_back("SERVER_PROTOCOL=" + m_path.substr(pos, m_path.find("\r\n") - pos));
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

    pos = path.find('?');
    if (pos != std::string::npos)
        path.erase(pos);

    if (access(path.c_str(), X_OK) == 0)
        throw std::logic_error("No Premissions Exception!");

    m_envp = this->AllocateEnviroment();
    m_argv = this->AllocateArgumentVector();

    if (pipe(pipefds) == PIPE_ERROR)
    {
        this->DeletePointerArray(this->m_envp, this->m_enviroment_var.size());
        throw std::logic_error("Pipe Failed Exception!");
    }

    pid = fork();
    if (pid == FORK_ERROR)
    {
        close(pipefds[READ]);
        close(pipefds[WRITE]);
        this->DeletePointerArray(this->m_envp, this->m_enviroment_var.size());
        throw std::logic_error("Fork Failed Exception!");
    }
    if (pid == CHILD)
    {
        log("Child is executing script", L_Info);
        close(pipefds[READ]);
        dup2(pipefds[WRITE], STDOUT_FILENO);
        close(pipefds[WRITE]);
        execve("/usr/bin/python3", m_argv, m_envp); // This needs to be changed if we are going to support multiple code langs.
        exit(1);
    }
    close(pipefds[WRITE]);
    this->DeletePointerArray(this->m_envp, this->m_enviroment_var.size());
    this->DeletePointerArray(this->m_argv, 2);
    waitpid(pid, NULL, 0);
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
            enviroment[index] = new char[(*it).size() + 1];
            strcpy(enviroment[index], (*it).c_str());
            index++;
        }
        enviroment[index] = NULL;
    }
    catch(const std::exception& e)
    {
        if (enviroment != NULL)
            this->DeletePointerArray(enviroment, index);
        throw; // rethrows original exception bad_alloc
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

        argv[index] = new char[m_path.size() + 1];
        strcpy(argv[1], m_path.c_str());
        index++;

        argv[index] = NULL;
    }
    catch(const std::exception& e)
    {
        if (argv != NULL)
            this->DeletePointerArray(argv, index);
        throw;
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
