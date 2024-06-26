#include "Response.hpp"
#include "CGI.hpp"
#include "constants.hpp"
#include"ResponseHelpers.hpp"
#include"HelperFuncs.hpp"

#define READ_ONLY std::ios::in
#define ERROR -1

Response::~Response()
{

}
/**
 * @brief Construct a new Response object
 *
 * @param client_request
 */


Response::Response(std::shared_ptr<Request> client_request) : m_client_request(client_request)
{
	m_status = StatusCode::Null;
	m_CGI = false;
}

/**
 * @brief create a response based on a status code
*/
Response::Response(int status_code)
{
	m_body = customizeErrorPage(status_code);
	addHeader(status_code);
}

void	Response::addHeader(int status_code)
{
	m_total_response = "HTTP/1.1 ";
	m_total_response.append(std::to_string(static_cast<int>(status_code)) + " " + m_DB_status.at(status_code) + "\r\n");
	m_total_response.append("Content-Length: " + std::to_string(m_body.size()) + "\r\n");
	m_total_response.append("Content-Type: text/html\r\n\r\n");
	m_total_response.append(m_body);
}

std::string	Response::customizeErrorPage(int status_code)
{
    std::string errorPage = DEFAULT_ERR_PAGE;
	errorPage.replace(errorPage.find("Oops!"), 5, std::to_string(status_code) + " " + m_DB_status.at(status_code));
    return errorPage;
}

/**
 * @brief Creates a response to send to the client based on the client's request.
 *
 * This method creates a response to send to the client based on the client's request (`m_client_request`).
 * It first checks if there is a redirection path in the client's request. If there is, it creates a redirection response and returns.
 * Otherwise, it extracts the HTTP method and the final path from the client's request.
 * It then checks if the file specified by the final path exists. If it doesn't, it sets the response status to `NotFound` and throws a `logic_error` exception.
 * If the file exists and its extension is "cgi" or "py", it sets `m_CGI` to true and executes the CGI script.
 * If the file exists but its extension is not "cgi" or "py", it performs an action based on the HTTP method: if the method is `GET`, it gets the file; if the method is `POST`, it uploads the file.
 *
 * @param server A pointer to the server object.
 * @throws std::logic_error If the file specified by the final path does not exist.
 */
void Response::createResponse(Server *server)
{
	std::fstream file;

	m_server = server;

	if (m_client_request->getRedirPath() != "")
	{
		createRedirect();
		return ;
	}
	m_method = m_client_request->getMethod();
	m_path = m_client_request->getFinalPath();

	try
	{
		if (this->m_client_request->getAutoindex()) // implement directory listing!!
		{
			if (std::filesystem::is_directory(m_path))
			{
				respondWithDirectoryListing();
				this->addHeader();
				return ;
			}
			else // path is not a directory
			{
				m_status = StatusCode::NotFound;
				file = this->OpenFile(m_client_request->getLocation().getErrorPageForCode(404));
				this->ReadFile(file);
				throw std::logic_error("File Not Found 404");
			}
		}

		if (!this->DoesFileExists()) // You can change here if we have a 404 not found page inside the config.
		{
			m_status = StatusCode::NotFound;
			throw std::logic_error("File Not Found 404");
		}
		if (this->ExtensionExtractor(m_path) == "cgi" || this->ExtensionExtractor(m_path) == "py")
		{
			log("CGI HAS BEEN CALLED");
			m_CGI = true;
			this->ExecuteCGI();
		}

		else
		{
			switch (m_method)
			{
			case HTTPMethod::GET:
				this->GetFile();
				break;
			case HTTPMethod::POST:
				this->UploadFile();
				break;
			case HTTPMethod::DELETE:
				this->DeleteFile();
				break;
			default:
				log("Unsopported Method Passed! Response!", L_Error);
				m_status = StatusCode::InternalServerError;
				break;
			}
		}

	}
	catch(const std::exception& e)
	{
		if (m_status == StatusCode::Null)
			m_status = StatusCode::InternalServerError;
		log(e.what(), L_Error);
	}

	this->addHeader();
}


/**
 * @brief This method will open and read the file requested by the Client
 *
 */
void Response::GetFile()
{
	std::fstream file;

	file = this->OpenFile(m_path);
	this->ReadFile(file);
}

/**
 * @brief This method will open a file check if the file has been succesfully opened, and returns a fstream to the file.
 *
 * @return std::fstream
 */
std::fstream Response::OpenFile(const std::string &path) noexcept(false)
{
	std::fstream file;

	file.open(path, READ_ONLY);
	if (!file.is_open())
	{
		m_status = StatusCode::Forbidden;
		throw std::logic_error("Forbidden File 403");
	}
	return file;
}

/**
 * @brief Constructs the total response with all the gathered and generated information.
 *
 * This method constructs the total response by appending the HTTP version, status code, status message, headers, and body.
 * If the status code is `Null`, it is set to `OK`.
 * The `Content-Length` and `Content-Type` headers are added based on the response body and the file extension of the path, respectively.
 * If the response is not from a CGI script, an additional blank line is appended to separate the headers from the body.
 * If the response is from a CGI script, the `Content-Length` is adjusted accordingly.
 * Finally, the response body is appended to the total response.
 */
void	Response::addHeader()
{
	size_t pos;
	std::fstream file;
	std::string request;

	request = m_client_request->getRequest();
	pos = request.find("HTTP");
	m_total_response.append(request.substr(pos, request.find(CRLF) - pos) + " ");
	if (m_status == StatusCode::Null)
		m_status = StatusCode::OK;
	m_total_response.append(std::to_string(static_cast<int>(m_status)) + " " + m_DB_status.at(static_cast<int>(m_status)) + CRLF);
	
	try
	{
		if (m_status != StatusCode::OK)
		{
			file = this->OpenFile(m_client_request->getLocation().getErrorPageForCode(static_cast<int>(m_status)));
			this->ReadFile(file);	
		}
	}
	catch(const std::exception& e)
	{
		log("Loading Error Page Went Wrong Exception!!", L_Error);
	}

	if (m_CGI == false)
	{
		m_total_response.append("Content-Length: " + std::to_string(m_body.size()) + CRLF);
		try
		{
			if (m_client_request->getAutoindex())
				m_total_response.append("Content-Type: " + m_DB_ContentType.at("html") + CRLF);
			else
				m_total_response.append("Content-Type: " + m_DB_ContentType.at(ExtensionExtractor(m_path)) + CRLF);
		}
		catch(const std::exception& e)
		{
			m_total_response.append("Content-Type: text/plain\r\n");
		}
		m_total_response.append(CRLF);
	}
	else
	{
		request = m_body;
		request.erase(0, request.find(CRLF) + 2);
		log("\n" + request, L_Info);
		m_total_response.append("Content-Length: " + std::to_string(request.size() - 1) + CRLF);
	}

	m_total_response.append(m_body);
	log(m_total_response, L_Info);

}


/**
 * @brief Reads the content of a file and appends it to the response body.
 *
 * This method reads the content of a file line by line and appends each line to the response body (`m_body`).
 * If it encounters an error before reaching the end of the file (i.e., if `file.eof()` is false), it sets the response status to `InternalServerError` and throws a `logic_error` exception.
 * After reading the file, it closes the file.
 *
 * @param file A reference to the file stream object associated with the file to read.
 * @throws std::logic_error If an error occurs while reading the file.
 */
void Response::ReadFile(std::fstream &file) noexcept(false)
{
	std::string line;

	while (std::getline(file, line))
		m_body.append(line + '\n');

	if (!file.eof())
	{
		m_status = StatusCode::InternalServerError;
		throw std::logic_error("Error Reading File 501");
	}
	file.close();
}

/**
 * @brief Executes a CGI script and sets the response body to the output of the script.
 *
 * This method executes a CGI script specified by the path. It sets up the environment variables required by the CGI script, forks a new process to execute the script, and captures the output of the script.
 * The output of the script is then set as the response body (`m_body`).
 * If an error occurs during the execution of the script, it sets the response status to `InternalServerError` and throws a `logic_error` exception.
 *
 * @throws std::logic_error If an error occurs while executing the CGI script.
 */
void Response::ExecuteCGI() noexcept(false)
{
	int 	fd;
	int 	bytes_read;
	char 	buffer[BUFFER_SIZE];
	CGI 	common_gateway_interface(m_client_request);

	try
	{
		fd = common_gateway_interface.ExecuteScript(m_path);
		bytes_read = read(fd, buffer, BUFFER_SIZE);
		while (bytes_read != 0)
		{
			std::string str(buffer, bytes_read);
			m_body.append(str);
			bytes_read = read(fd, buffer, BUFFER_SIZE);
		}
	}
	catch(StatusCode &status)
	{
		m_status = status;
		close(fd);
		
	}
	log("Done reading CGI PIPE", L_Info);
	close(fd);
}

bool Response::DoesFileExists()
{
	if (!std::filesystem::exists(m_path))
		return false;

	if (std::filesystem::is_directory(m_path) && m_method == HTTPMethod::GET)
		return false;
	return true;
}

std::string	Response::ExtensionExtractor(const std::string &path)
{
	std::string line;

	line = path.substr(path.find('.') + 1);
	if (path.find('.') == line.npos && m_status == StatusCode::OK)
		line = "txt";
	else if (path.find('.') == line.npos)
		line = "html";
	return line;
}

void Response::createRedirect()
{
	Location loc = m_client_request->getLocation();
	int redir_status_code = loc.getReturnStatusCode();
	m_total_response = "HTTP/1.1 ";
	m_total_response.append(std::to_string(static_cast<int>(redir_status_code)) + " " + m_DB_status.at(static_cast<int>(redir_status_code)));
	m_total_response += CRLF;
	m_total_response += "Location: " + loc.getReturnBody() + CRLFCRLF;
}

const std::string &Response::getResponse() const
{
	return m_total_response;
}

/**
 * @brief Deletes a file from the server.
 *
 * This method deletes a file from the server. The path of the file to delete is stored in `m_path`.
 * If the file deletion is successful, it appends "File Deleted Successfully" to the response body (`m_body`) and sets the response status to `NoContent`.
 * If an error occurs during the file deletion process, it appends "File Deleted Unsuccessful! [ERROR]" to the response body, sets the response status to `InternalServerError`, and throws a `logic_error` exception.
 *
 * @throws std::logic_error If an error occurs while deleting the file.
 */
void	Response::DeleteFile() noexcept(false)
{
	if (!std::filesystem::remove(m_path))
	{
		m_body.append("File Deleted Unseccesfull! [ERROR]");
		m_status = StatusCode::InternalServerError;
		throw std::logic_error("DeleteFile: Failed to delete File!");
	}
	m_body.append("File Deleted Succesfully");
	log("File Deleted Succesfully!", L_Info);
	m_status = StatusCode::NoContent;
}

/**
 * @brief Uploads a file to the server.
 *
 * This method uploads a file to the server. It extracts the filename and the file content from the request body, creates a new file with the extracted filename in the `www/upload/` directory, and writes the extracted file content to the new file.
 * If an error occurs during the file creation or writing process, it sets the response status to `Forbidden` and throws a `logic_error` exception.
 *
 * @throws std::logic_error If an error occurs while creating or writing to the file.
 */
void	Response::UploadFile() noexcept(false)
{
	int fd;
	size_t pos;
	std::string body;
	std::string filename;
	std::string request_body;
	std::string boundary;

	request_body = m_client_request->getBody();

	pos = request_body.find("filename");
	pos += 10; // Skip over [filename="]
	filename = request_body.substr(pos, request_body.find(CRLF, pos) - (pos + 1));
	
	pos = request_body.find(CRLF); // Find the boundary of the form data. (example: [-----------------------------114782935826962])
	boundary = request_body.substr(0, pos);

	pos = request_body.find(CRLFCRLF);
	pos += 4; // Skip over [\r\n\r\n]
	body = request_body.substr(pos, request_body.find(boundary, pos) - (pos + 2));


	fd = open((std::string("www/upload/") + filename).c_str(), O_CREAT | O_RDWR, 0666);
	if (fd == ERROR)
	{
		m_status = StatusCode::Forbidden;
		throw std::logic_error("Open: ERROR");
	}

	WriteToFile(fd, body); /* @warning needs to be changed when we implement Poll Writing! */
	close(fd);
	m_body = "Uploaded File!";
}

void	Response::WriteToFile(int fd, const std::string &buffer) noexcept(false)
{
	write(fd, buffer.c_str(), buffer.size());
}

void Response::respondWithDirectoryListing()
{
	std::string html_str = createautoIndexHTML(m_path);
	for (const auto& entry : std::filesystem::directory_iterator(m_path)) {
		std::string filename = entry.path().filename().string();
		if (filename.find('.') == std::string::npos)
			filename.append("/");
		html_str +=  "<li><a href=\"" + filename + "\">" + filename + "</a></li>\n";
	}
	html_str += HTML_FOOTER;
	m_body = html_str;
}
