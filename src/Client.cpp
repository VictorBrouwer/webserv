#include"Client.hpp"
#include"HelperFuncs.hpp"

// const int BUFFER_SIZE = 30720;

Client::Client(int socket) : m_socket(socket), m_request(std::make_shared<Request>()), m_total_bytes_sent(0), m_server(nullptr)
{
	Response response(this->m_request);
	m_response = std::make_shared<Response>(response);
	std::cout << "Client created\n";
}

Client::~Client()
{
}

ClientState & Client::getState()
{
	return this->m_state;
}

void	Client::sendResponse()
{
	int bytes_sent;

	// write(this->m_socket, m_response.getResponse().c_str(), m_response.getResponse().size());
	// log(std::string("sending response: " + m_response->getResponse()), L_Info);
	bytes_sent = send(this->m_socket, m_response->getResponse().c_str(), m_response->getResponse().size(), 0);
	if (bytes_sent < 0)
	{
		m_state = ClientState::ERROR;
		return;
	}
	m_total_bytes_sent += bytes_sent;
	if (m_total_bytes_sent == this->m_response->getResponse().length())
	{
		if(m_request->Get_Keep_Alive() == true)
		{
			m_request.reset(new Request);
			m_response.reset(new Response(m_request));
			m_state = ClientState::KEEP_ALIVE;
			m_total_bytes_sent = 0;
		}
		else
			m_state = ClientState::REMOVE_CONNECTION;
	}
	else if (m_total_bytes_sent < this->m_response->getResponse().length())
		m_state = ClientState::SENDING;
}

void	Client::receive(std::vector<Server> &servers)
{
	m_state = m_request->readFromClient(m_socket);
	// log("\n" + m_request->Get_Request(), L_Info);
	// m_state is either loading or reading_done
	// if client state is loading, the poll event should remain POLLIN
	// if client statis done_reading, a response should be created and then 
	// the poll event should be set to pollout
	// pollout basically tells you that the sending will succeed
	switch (m_state)
	{
	case ClientState::ERROR:
		break;
	case ClientState::LOADING:
		break;
	case ClientState::READING_DONE:
		this->extractServer(servers);
		this->checkRequestSyntax(m_request->Get_Request());
		m_request->handleLocation(m_server);
		m_response->createResponse(m_server);
		m_state = ClientState::READY_TO_SEND; // maybe set this somewhere else
		break;
	default:
		break;
	}
}

void Client::checkRequestSyntax(const std::string& request)
{
   std::istringstream iss(request);
   std::vector<std::string> lines;
   std::string line;
   while (std::getline(iss, line))
	   lines.push_back(line);
	// Check the minimum number of lines
   if (lines.size() < 3)
		throw std::runtime_error("invalid HTTP-request");
	const std::string& request_line = lines[0];
	std::istringstream request_line_stream(request_line);
	std::string method, path, http_version;
	request_line_stream >> method >> path >> http_version;
	if (path[0] != '/')
		throw std::runtime_error("invalid HTTP-request");
	if (http_version != "HTTP/1.1")
		throw std::runtime_error("invalid HTTP-request");
}

void	Client::extractServer(std::vector<Server> &servers)
{
	for (auto server : servers)
	{
		for (const auto& servername : server.getServerNames())
		{
			if (servername == this->m_request->extractHostPort(HostPort::HOST))
				m_server = &server;
		}
	}
	if (!m_server)
		m_server = &(*(servers.begin()));
		// m_server = servers->data();
}