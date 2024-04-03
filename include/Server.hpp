#pragma once

#include "Directive.hpp"
#include "Location.hpp"
#include "ConfigShared.hpp"
#include "ConfigReturn.hpp"
#include "Logger.hpp"
#include <memory>

class Server : public ConfigShared, public ConfigReturn {
	public:
		Server(const Directive &server_directive, ConfigShared* config,
				const Logger& logger);

		const std::vector<std::pair<std::string, int>>& getListens( void ) const;

		~Server();
		const std::vector<std::string>& getServerNames();
		Location* findLocation(const std::string &uri);

	private:
		// int m_socket;

		std::vector<std::string>                 server_names;
		std::vector<Location>                    locations;
		std::vector<std::pair<std::string, int>> listens;

		// Socket file descriptors that this server could potentially respond to
		std::vector<int> sockets;

		Logger l;

		void applyServerNameDirective(const Directive& directive);
		void applyListenDirective(const Directive& directive);
};
