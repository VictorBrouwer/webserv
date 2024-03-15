#include "Server.hpp"
#include "HelperFuncs.hpp"

Server::Server(int socket) : m_socket(socket)
{
	log("Server created\n");
	(void) m_socket;
}

Server::Server(const Directive &server_directive, ConfigShared* config, const Logger& logger) : ConfigShared(config), l("Server", logger.getLogLevel()) {
	// Set logger context with a recognizable name if possible,
	// else keep the default of "Server"
	try {
		this->l.setDefaultContext("Server " + server_directive.getPrimaryServerName());
	}
	catch(const std::invalid_argument& e) {	}

	l.log("Setting up server");

	// Options for ConfigShared
	this->applySharedDirectives(server_directive.getSubdirectives(), l);

	// Specific options for server: server_name, return, listen
	l.log("Applying server-specific config options");

	auto start = server_directive.getSubdirectivesIterator();
	auto end   = server_directive.getSubdirectivesEnd();
	auto it    = start;

	it = std::find(start, end, "server_name");
	if (it != end) {
		this->applyServerNameDirective(*it);
	}

	it = std::find(start, end, "return");
	if (it != end) {
		this->applyReturnDirective(*it);
	}

	// Listen directive
	it = start;
	while (it != end) {
		if (*it == "listen")
			this->applyListenDirective(*it);
		++it;
	}
}

Server::~Server() { }

void Server::applyServerNameDirective(const Directive& directive) {
	auto it  = directive.getArgumentsIterator();
	auto end = directive.getArgumentsEnd();

	while (it != end) {
		// With a directive like ".example.com" we need to add both
		// a wildcard version and a default version to the server_names.
		if ((*it)[0] == '.') {
			std::string edit = "*" + *it;
			this->server_names.push_back(edit);

			edit.erase(0, 2);
			this->server_names.push_back(edit);
		} else {
			this->server_names.push_back(*it);
		}

		++it;
	}
}

void Server::applyListenDirective(const Directive& directive) {
	// Split into interface and port. If not given, interface is 0.0.0.0
	// and port is 80 or 8080 depending on whether we run as root.

	try {
		const std::string&          input  = directive.getArguments()[0];
		std::pair<std::string, int> output = {"0.0.0.0", 80};

		std::size_t divider_pos = input.find(':');
		if (divider_pos != std::string::npos) {
			// If there's a : in the input, we have an interface and a port
			output.first  = input.substr(0, divider_pos);
			output.second = std::stoi(input.substr(input.find(':') + 1));
		}
		else {
			// Figure out whether we are dealing with an interface or a port
			// and assume the other to be default.
			if (input.find_first_not_of("0123456789") == std::string::npos) {
				// Argument is purely numeric, we have a port
				output.second = std::stoi(input);
			}
			else {
				// Argument has other characters, we have a hostname
				output.first = input;
			}
		}

		// Apply the interface/port combo to our vector
		this->listens.push_back(output);
	}
	catch(const std::exception& e) {
		l.log("Unexpected error occurred when adding listen directive: " + std::string(e.what()), L_Error);
		throw;
	}
}
