#include "Response.hpp"

#define READ_ONLY std::ios::in
#define WRITE_ONLY std::ios::out

Response::Response(Request &client_request) : m_client_request(client_request), m_headers(client_request.Get_Headers())
{
	m_status = StatusCode::Null;
}

Response::~Response()
{

}
// Step 1 Recognise wich HTTP method request we got (done).
// Step 2 Lookup PATH (does it exist, do you have premissions(Write, Read (Only Read with GET))) (done)
// Step 3 Read the File (Keep the amount of characters read (Content-Length = character read)) 
// Step 4 Create Header If no status code has been set set status to 200 (OK)
// Step 5 Make a function that fills additional information (Set this to template)
// Step 6 Copy the requested information in the (Body)
void Response::createResponse()
{
	log("Response Recieved");
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

bool Response::DoesFileExists()
{
	log(m_client_request.Get_Path());
	if (std::filesystem::exists(m_client_request.Get_Path()))
	{
		m_status = StatusCode::NotFound;
		return false;
	}

	return true;
}
// Make a map for the status values 200 -> OK
void	Response::addHeader()
{
	size_t pos;
	std::string request;

	request = m_client_request.Get_Request();
	pos = request.find("HTTP");
	m_total_response.append(request.substr(pos, request.find("\r\n") - pos));


}

std::fstream Response::OpenFile(std::ios_base::openmode mode) noexcept(false)
{
	std::fstream file;

	if (this->DoesFileExists())
		throw std::logic_error("File Not Found 404");

	file.open(m_client_request.Get_Path(), mode);
	if (!file.is_open())
	{
		m_status = StatusCode::Forbidden;
		throw std::logic_error("Forbidden File 403");
	}

	return file;
}

void Response::ReadFile(std::fstream &file) noexcept(false)
{
	std::string line;

	while (std::getline(file, line))
	{
		log(line);
		m_body.append(line);
	}
	if (!file.eof())
	{
		m_status = StatusCode::InternalServerError;
		throw std::logic_error("Error Reading File 501");
	}

	m_content_length = m_body.size();
}

void Response::Get_Response() 
{
	std::fstream file;
	
	try
	{
		file = this->OpenFile(READ_ONLY);
		this->ReadFile(file);
		this->addHeader();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
}