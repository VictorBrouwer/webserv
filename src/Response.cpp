#include "Response.hpp"

#define READ_ONLY std::ios::in
#define WRITE_ONLY std::ios::out

Response::~Response()
{

}

Response::Response(Request &client_request) : m_client_request(client_request)
{
	m_status = StatusCode::Null;
	m_content_length = 0;
}

// Step 1 Recognise wich HTTP method request we got (done).
// Step 2 Lookup PATH (does it exist, do you have premissions(Write, Read (Only Read with GET))) (done)
// Step 3 Read the File (Keep the amount of characters read (Content-Length = character read))
// Step 4 Create Header If no status code has been set set status to 200 (OK)
// Step 5 Make a function that fills additional information (Set this to template)
// Step 6 Copy the requested information in the (Body)
void Response::createResponse()
{
	switch (m_client_request.Get_Method())
	{
	case HTTPMethod::GET:
		this->Get_Response();
		break;
	default:
		std::cout << "DELETE / POST (W.I.P)" << std::endl;
		break;
	}
}

void Response::Get_Response()
{
	std::fstream file;

	try
	{
		file = this->OpenFile(READ_ONLY);
		this->ReadFile(file);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	this->addHeader();

}

std::fstream Response::OpenFile(std::ios_base::openmode mode) noexcept(false)
{
	std::fstream file;

	if (!this->DoesFileExists()) // You can change here if we have a 404 not found page inside the config.
	{
		m_status = StatusCode::NotFound;
		throw std::logic_error("File Not Found 404");
	}

	file.open(m_client_request.Get_Path(), mode);
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

	request = m_client_request.Get_Request();
	pos = request.find("HTTP");
	m_total_response.append(request.substr(pos, request.find("\r\n") - pos) + " ");
	if (m_status == StatusCode::Null)
		m_status = StatusCode::OK;
	m_total_response.append(std::to_string(static_cast<int>(m_status)) + " " + m_DB_status.at(static_cast<int>(m_status)) + "\r\n");
	m_total_response.append("Content-length: " + std::to_string(m_content_length) + "\r\n");
	m_total_response.append("Content-type: " + m_DB_ContentType.at(ExtensionExtractor(m_client_request.Get_Path())) + "\r\n");
	m_total_response.append("\r\n");
	m_total_response.append(m_body);

	log(m_total_response);
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

	m_content_length = m_body.size();
}

bool Response::DoesFileExists()
{
	if (!std::filesystem::exists(m_client_request.Get_Path()))
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

const std::string &Response::getResponse() const
{
	return m_total_response;
}
