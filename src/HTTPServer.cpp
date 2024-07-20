#include <algorithm>
#include <cerrno>
#include <cstring>

#include "HTTPServer.hpp"
#include "Configuration.hpp"
#include "Directive.hpp"
#include "Socket.hpp"

HTTPServer::HTTPServer(Configuration &config, const Logger& logger) : ConfigShared(), l("HTTPServer", logger.getLogLevel())
{
	try {
		l.log("Constructing HTTPServer.", L_Info);

		l.log("Checking for http directive");
		const Directive& http_directive = config.getHttpDirective();

		this->applySharedDirectives(http_directive.getSubdirectives(), l);

		std::vector<Directive>::const_iterator it  = http_directive.getSubdirectivesIterator();
		std::vector<Directive>::const_iterator end = http_directive.getSubdirectivesEnd();

		l.log("Setting up virtual servers.", L_Info);
		while (it != end) {
			if (*it == "server")
				this->servers.push_back(Server(*it, (ConfigShared*) this, l));
			++it;
		}

		l.log("Setting up sockets.", L_Info);
		this->setupSockets();

		// Reserve space in the poll loop vectors for everything
		// we are going to do there
		this->poll_vector.reserve(1024);
		this->read_fd_pointers.reserve(1024);
		this->write_fd_pointers.reserve(1024);

		this->clients.reserve(1024);
	}
	catch(const std::exception& e) {
		l.log(e.what(), L_Error);
		throw; // Rethrow to catch in main
	}
}

HTTPServer::~HTTPServer() {
	// closeServer();
	std::for_each(this->getSocketIterator(), this->getSocketEnd(), [](Socket& s) {
		if (s.getFileDescriptor() != -1) {
			close(s.getFileDescriptor());
		}
	});
}

std::vector<Server>::iterator HTTPServer::getServerMutableIterator( void ) {
	return this->servers.begin();
}

std::vector<Server>::iterator HTTPServer::getServerMutableEnd( void ) {
	return this->servers.end();
}

std::vector<Server>& HTTPServer::getServerVector( void ) {
	return this->servers;
}

std::vector<Socket>::iterator HTTPServer::getSocketIterator( void ) {
	return this->sockets.begin();
}

std::vector<Socket>::iterator HTTPServer::getSocketEnd( void ) {
	return this->sockets.end();
}

std::vector<Client>::iterator HTTPServer::getClientIterator( void ) {
	return this->clients.begin();
}

std::vector<Client>::iterator HTTPServer::getClientEnd( void ) {
	return this->clients.end();
}

// Collects all sockets to open for the virtual servers and binds them
// to their addresses. Populates the sockets vector in HTTPServer.
void HTTPServer::setupSockets( void ) {
	l.log("Collecting required sockets.");
	std::unordered_map<int, std::vector<std::string>> sockets_requested;

	auto start = this->getServerMutableIterator();
	auto end   = this->getServerMutableEnd();
	auto it    = start;

	// Collect all listen directives in our map, sorted by port
	while (it != end) {
		auto listens = it->getListens();
		std::for_each(
			listens.begin(), listens.end(),
			[&](const std::pair<std::string, int>& p) {
				sockets_requested[p.second].push_back(p.first);
			}
		);
		++it;
	}

	std::vector<std::pair<std::string, int>> sockets_to_open;

	// Check which sockets need to be opened for each port
	// If we have 0.0.0.0 or *, open that, else open each unique one individually
	std::for_each(
		sockets_requested.begin(), sockets_requested.end(),
		[&](const std::pair<const int, std::vector<std::string>>& p) {
			if (std::find(p.second.begin(), p.second.end(), "*") != p.second.end() ||
				std::find(p.second.begin(), p.second.end(), "0.0.0.0") != p.second.end()) {
				sockets_to_open.push_back({"0.0.0.0", p.first});
			} else {
				std::for_each(p.second.begin(), p.second.end(), [&](const std::string& interface) {
					sockets_to_open.push_back({interface, p.first});
				});
			}
		}
	);

	l.log("Done. Amount of sockets to open: " + std::to_string(sockets_to_open.size()));
	l.log("Opening sockets...");
	std::for_each(sockets_to_open.begin(), sockets_to_open.end(), [&](const std::pair<std::string, int>& s) {
		this->sockets.emplace_back(s.first, s.second, l, this->clients);
	});

	l.log("Done. Sockets are ready for listening.");
}

void HTTPServer::startListening( void ) {
	l.log("Calling startListening() on all sockets.");
	std::for_each(this->getSocketIterator(), this->getSocketEnd(), [](Socket& s) {
		s.startListening();
	});
}

void HTTPServer::doPollLoop( void ) {
	l.log("Starting poll loop.");
	this->assemblePollQueue();
	this->runPoll();
	this->cleanUpPoll();
}

// Grab all ReadFileDescriptor and WriteFileDescriptor instances and
// assemble a queue out of them
void HTTPServer::assemblePollQueue( void ) {
	// Clear whatever is in the queue from before
	this->poll_vector.clear();
	this->read_fd_pointers.clear();
	this->write_fd_pointers.clear();

	// Add all listening sockets
	std::for_each(this->getSocketIterator(), this->getSocketEnd(), [&](Socket& socket) {
		this->addReadFileDescriptorToPoll((ReadFileDescriptor*) &socket);
	});

	// Add all Clients that want to keep reading/writing
	std::for_each(this->clients.begin(), this->clients.end(), [&](Client& client) {
		// Add a read file decsriptor if we're polling the client for the request
		if (client.getReadFDStatus() == FD_POLLING)
			this->addReadFileDescriptorToPoll((ReadFileDescriptor*) &client);
		// Add a write file descriptor if we're polling the client for the response
		if (client.getWriteFDStatus() == FD_POLLING)
			this->addWriteFileDescriptorToPoll((WriteFileDescriptor*) &client);

		std::shared_ptr<Response> &response = client.getResponse();
		if (response.get()) {
			// Add a read file descriptor if the response needs to load a file
			if (response->getReadFDStatus() == FD_POLLING)
				this->addReadFileDescriptorToPoll((ReadFileDescriptor*) response.get());
			// Add a write file descriptor if the response needs to upload something
			if (response->getWriteFDStatus() == FD_POLLING)
				this->addWriteFileDescriptorToPoll((WriteFileDescriptor*) response.get());
		}
	});
}

static std::string formatRevents(const struct pollfd& poll_fd) {
    std::string result;
    result += (poll_fd.revents & POLLIN)     ? " POLLIN"     : "";
    result += (poll_fd.revents & POLLOUT)    ? " POLLOUT"    : "";
    result += (poll_fd.revents & POLLHUP)    ? " POLLHUP"    : "";
    result += (poll_fd.revents & POLLNVAL)   ? " POLLNVAL"   : "";
    result += (poll_fd.revents & POLLPRI)    ? " POLLPRI"    : "";
    result += (poll_fd.revents & POLLRDBAND) ? " POLLRDBAND" : "";
    result += (poll_fd.revents & POLLRDNORM) ? " POLLRDNORM" : "";
    result += (poll_fd.revents & POLLWRBAND) ? " POLLWRBAND" : "";
    result += (poll_fd.revents & POLLWRNORM) ? " POLLWRNORM" : "";
    result += (poll_fd.revents & POLLERR)    ? " POLLERR"    : "";
    return result;
}

// Run the poll call on the queue as it has been assembled
void HTTPServer::runPoll( void ) {
	pollfd* poll_data  = this->poll_vector.data();
	nfds_t  poll_count = this->poll_vector.size();

	l.log("Running poll with " + std::to_string(poll_count) + " file descriptors.", L_Info);
	l.log("Current clients: " + std::to_string(this->clients.size()), L_Info);

	int poll_return = poll(poll_data, poll_count, 10000);
	if (poll_return < 0) {
		throw HTTPServer::Exception("poll error: " + std::string(std::strerror(errno)));
	}
	else if (poll_return == 0) {
		l.log("No events returned this cycle.", L_Info);
	}
	else {
		l.log("Events returned: " + std::to_string(poll_return), L_Info);
		this->handleEvents();
	}
}

// For each event returned by runPoll, perform the required action
void HTTPServer::handleEvents( void ) {
	std::for_each(this->poll_vector.begin(), this->poll_vector.end(), [&](pollfd pollfd) {
		if (pollfd.revents != 0) {
			l.log("File descriptor " + std::to_string(pollfd.fd) + " has these events:" + formatRevents(pollfd), L_Info);

			if (pollfd.revents & POLLIN) {
				this->read_fd_pointers[pollfd.fd]->readFromFileDescriptor(pollfd);
			}

			if (pollfd.revents & POLLOUT) {
				this->write_fd_pointers[pollfd.fd]->writeToFileDescriptor();
			}

			// We've been hung up but we wanted to write more, so we need to
			// make sure writingDone is called separately.
			if (pollfd.revents & POLLHUP) {
				if (this->write_fd_pointers.contains(pollfd.fd)) {
					this->write_fd_pointers[pollfd.fd]->setWriteFDStatus(FD_HUNG_UP);
					this->write_fd_pointers[pollfd.fd]->callWritingDone();
				}
				// We want to finish reading before closing in case that's required.
				if ((pollfd.revents & POLLIN) == 0 && this->read_fd_pointers.contains(pollfd.fd)) {
					this->read_fd_pointers[pollfd.fd]->setReadFDStatus(FD_HUNG_UP);
					this->read_fd_pointers[pollfd.fd]->callReadingDone();
				}
			}
		}
	});
}

void HTTPServer::addReadFileDescriptorToPoll(ReadFileDescriptor* read_fd) {
	auto existing_fd = std::find_if(this->poll_vector.begin(), this->poll_vector.end(), [&](pollfd pollfd) {
		return read_fd->getReadFileDescriptor() == pollfd.fd;
	});

	if (existing_fd != this->poll_vector.end()) {
		// Add read capabilities to this file descriptor
		existing_fd->events  = existing_fd->events | 1 << POLLIN;
	}
	else {
		// Add a new file descriptor with read capabilities
		this->poll_vector.push_back({read_fd->getReadFileDescriptor(), POLLIN, 0});
	}

	// Add the pointer to the vector as well to be able to find it back later
	this->read_fd_pointers.insert({read_fd->getReadFileDescriptor(), read_fd});
}

void HTTPServer::addWriteFileDescriptorToPoll(WriteFileDescriptor* write_fd) {
	auto existing_fd = std::find_if(this->poll_vector.begin(), this->poll_vector.end(), [&](pollfd pollfd) {
		return write_fd->getWriteFileDescriptor() == pollfd.fd;
	});

	if (existing_fd != this->poll_vector.end()) {
		// Add write capabilities to this file descriptor
		existing_fd->events  = existing_fd->events | 1 << POLLOUT;
	}
	else {
		// Add a new file descriptor with write capabilities
		this->poll_vector.push_back({write_fd->getWriteFileDescriptor(), POLLOUT, 0});
	}

	// Add the pointer to the vector as well to be able to find it back later
	this->write_fd_pointers.insert({write_fd->getWriteFileDescriptor(), write_fd});
}

void HTTPServer::cleanUpPoll( void ) {
	std::erase_if(this->clients, [&](Client& client) {
		return (client.getWriteFDStatus() == FD_DONE || client.getWriteFDStatus() == FD_HUNG_UP);
	});
}
