#include "Response.hpp"


Response::Response(Request &client_request) : m_client_request(client_request)
{
	m_status = StatusCode::Null;
}

Response::~Response()
{

}

void Response::createResponse()
{
	switch (m_client_request.Get_Method())
	{
	case HTTPMethod::GET :
		this->Get_Response();
		break;
	default:
		std::cout << "DELETE / POST (W.I.P)" << std::endl;
		break;
	}

}

void Response::Get_Response()
{
	
}