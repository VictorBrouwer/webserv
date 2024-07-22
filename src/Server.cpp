#include "Server.hpp"
#include "HelperFuncs.hpp"

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

	auto start = server_directive.getSubdirectivesIterator();
	auto end   = server_directive.getSubdirectivesEnd();
	auto it    = start;

	// Options for ConfigReturn
	it = std::find(start, end, "return");
	if (it != end) {
		this->applyReturnDirective(*it);
	}

	// Options specifically for servers: location, server_name and listen
	l.log("Applying server-specific config options");

	it = std::find(start, end, "server_name");
	if (it != end) {
		this->applyServerNameDirective(*it);
	}

	it = start;
	while (it != end) {
		if (*it == "listen")
			this->applyListenDirective(*it);
		++it;
	}

	l.log("Setting up locations for server", L_Info);
	it = start;
	while (it != end) {
		if (*it == "location") {
			this->locations.push_back(Location(*it, (ConfigShared*) this, l));
		}
		++it;
	}
}

const std::vector<std::pair<std::string, int>>& Server::getListens( void ) const {
	return this->listens;
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

bool Server::operator==(int file_descriptor) const {
	return std::find(this->sockets.begin(), this->sockets.end(), file_descriptor) != this->sockets.end();
}

Location* Server::findLocation(const std::string &uri) // to do: find the longest location match with uri
{
	Location *matching_loc = NULL;
	Location *default_loc;
    std::vector<std::string> path_comps = split(uri, "/");
    std::string basename;
	std::string location_dir;

	if (path_comps.size() == 1)
		basename = "";
	else
		basename = path_comps[1];
	for(auto &location : locations)
	{
		location_dir = location.getUri();
		if (location_dir == "/") {
			default_loc = &location;
			break;
		}
	}
	for(auto &location : locations)
	{
		location_dir = strip(location.getUri(), "/");
		if (location_dir == basename) {
            matching_loc = &location;
			break;
		}
    }
	if (matching_loc == NULL)
		matching_loc = default_loc;
	return matching_loc;
	// return &this->locations[0];
}


const std::vector<std::string>& Server::getServerNames()
{
	return server_names;
}

// checks if the server contains a given socket
bool Server::containsSocket(int socket_fd) const {
	std::vector<int> sockets = this->sockets;
	return std::find(this->sockets.begin(), this->sockets.end(), socket_fd) != this->sockets.end();
}
