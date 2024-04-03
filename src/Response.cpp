#include "Response.hpp"
#include "cgi.hpp"
#include "constants.hpp"

#define READ_ONLY std::ios::in
#define WRITE_ONLY std::ios::out
#define ERROR -1

Response::~Response()
{

}

Response::Response(std::shared_ptr<Request> client_request) : m_client_request(client_request)
{
	m_status = StatusCode::Null;
	m_CGI = false;
}

// Step 1 Recognise wich HTTP method request we got (done).
// Step 2 Lookup PATH (does it exist, do you have premissions(Write, Read (Only Read with GET))) (done)
// Step 3 Read the File (Keep the amount of characters read (Content-Length = character read))
// Step 4 Create Header If no status code has been set set status to 200 (OK)
// Step 5 Make a function that fills additional information (Set this to template)
// Step 6 Copy the requested information in the (Body)
void Response::createResponse(Server *server)
{
	m_server = server;
	if (m_client_request->Get_redir_path() != "")
	{
		createRedirect();
		return ;
	}
	m_method = m_client_request->Get_Method();
	m_path = m_client_request->Get_final_path();
	switch (m_method)
	{
	case HTTPMethod::GET:
		this->ParseResponse(READ_ONLY);
		break;
	case HTTPMethod::POST:
		this->ParseResponse(READ_ONLY | WRITE_ONLY);
		break;
	case HTTPMethod::DELETE:
		this->ParseResponse(READ_ONLY | WRITE_ONLY);
		break;
	default:
		log("Unsopported Method Passed! Response!", L_Error);
		break;
	}
}

void Response::ParseResponse(std::ios_base::openmode mode)
{
	std::fstream file;

	try
	{
		if (!this->DoesFileExists()) // You can change here if we have a 404 not found page inside the config.
		{
			m_status = StatusCode::NotFound;
			throw std::logic_error("File Not Found 404");
		}
		log("Different 404", L_Info);
		if (m_method == HTTPMethod::DELETE)
			this->DeleteFile();

		if (m_method == HTTPMethod::POST)
			this->UploadFile();

		if (this->ExtensionExtractor(m_path) == "cgi" || this->ExtensionExtractor(m_path) == "py")
		{
			m_CGI = true;
			this->ExecuteCGI();
		}
		else if (m_method == HTTPMethod::GET)
		{
			file = this->OpenFile(mode);
			this->ReadFile(file);
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

std::fstream Response::OpenFile(std::ios_base::openmode mode) noexcept(false)
{
	std::fstream file;

	file.open(m_path, mode);
	if (!file.is_open())
	{
		m_status = StatusCode::Forbidden;
		throw std::logic_error("Forbidden File 403");
	}
	return file;
}

// Make a map for the status values 200 -> OK
// Header HTTP/[VER] [CODE] [TEXT]
// Header Conent-Length [Length-Of-Body]
// "\r\n"
// Body [file contents]
void	Response::addHeader()
{
	size_t pos;
	std::string request;

	request = m_client_request->Get_Request();
	pos = request.find("HTTP");
	m_total_response.append(request.substr(pos, request.find("\r\n") - pos) + " ");
	if (m_status == StatusCode::Null)
		m_status = StatusCode::OK;
	m_total_response.append(std::to_string(static_cast<int>(m_status)) + " " + m_DB_status.at(static_cast<int>(m_status)) + "\r\n");

	if (m_CGI == false)
	{
		m_total_response.append("Content-Length: " + std::to_string(m_body.size()) + "\r\n");
		m_total_response.append("Content-Type: " + m_DB_ContentType.at(ExtensionExtractor(m_path)) + "\r\n");
		m_total_response.append("\r\n");
	}
	else
	{
		request = m_body;
		request.erase(0, request.find("\r\n") + 2);
		log("\n" + request, L_Info);
		m_total_response.append("Content-Length: " + std::to_string(request.size() - 1) + "\r\n");
	}

	m_total_response.append(m_body);

}

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

// This function can be changed into one read useage!!
void Response::ExecuteCGI() noexcept(false)
{
	int 	fd;
	int 	bytes_read;
	char 	buffer[BUFFER_SIZE];
	cgi 	common_gateway_interface(m_client_request);

	try
	{
		fd = common_gateway_interface.ExecuteScript(m_path);
		bytes_read = read(fd, buffer, BUFFER_SIZE);
		while (bytes_read != 0)
		{
			std::string str(buffer);
			str.resize(bytes_read);
			m_body += str;
			bytes_read = read(fd, buffer, BUFFER_SIZE);
		}
	}
	catch(const std::exception& e)
	{
		m_status = StatusCode::InternalServerError;
		close(fd);
		throw;
	}
	log("Done reading CGI PIPE", L_Info);
	close(fd);
}

bool Response::DoesFileExists()
{
	log(m_path, L_Info);
	if (!std::filesystem::exists(m_path))
		return false;
	return true;
}

std::string	Response::ExtensionExtractor(const std::string &path)
{
	std::string line;

	line = path.substr(path.find('.') + 1);
	if (path.find('.') == line.npos)
		line = "txt";
	return line;
}

void Response::createRedirect()
{
	Location loc = m_client_request->Get_location();
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

void	Response::UploadFile() noexcept(false)
{
	int fd;
	size_t pos;
	std::string body;
	std::string filename;
	std::string request_body;

	request_body = m_client_request->Get_Body();
	
	pos = request_body.find("\r\n\r\n");
	pos += 4; // Skip over [\r\n\r\n]
	body = request_body.substr(pos, request_body.find("\r\n", pos) - (pos + 1));
	
	log("Response = Body[" + std::to_string(body.size()) + "]", L_Warning);

	pos = request_body.find("filename");
	// log(std::to_string(body.size()), L_Error);
	pos += 10; // Skip over [filename="]
	filename = request_body.substr(pos, request_body.find("\r\n", pos) - (pos + 1));

	fd = open((m_path + filename).c_str(), O_CREAT | O_RDWR, 0666);
	if (fd == ERROR)
	{
		m_status = StatusCode::Forbidden;
		throw std::logic_error("Open: ERROR");
	}

	WriteToFile(fd, body); /* @warning needs to be changed when we implement chunk reading!! */
	close(fd);
	m_body = "Uploaded File!";
}

void	Response::WriteToFile(int fd, const std::string &buffer) noexcept(false)
{
	size_t pos;
	std::string request_body = m_client_request->Get_Body();

	pos = request_body.find("\r\n\r\n");
	pos += 4;
	write(fd, buffer.c_str(), request_body.find("\r\n", pos) - (pos + 1));
}