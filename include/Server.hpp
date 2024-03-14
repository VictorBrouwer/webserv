#pragma once

#include "Directive.hpp"
#include "Location.hpp"
#include "ConfigShared.hpp"
#include "ConfigReturn.hpp"
#include "Logger.hpp"

class Server : public ConfigShared, public ConfigReturn {
	public:
		Server(int socket);
		Server(const Directive &server_directive, ConfigShared* config,
				const Logger& logger);
		~Server();

	private:
		int m_socket;

		std::vector<std::string>                 server_names;
		std::vector<Location>                    locations;
		std::vector<std::pair<std::string, int>> listens;

		Logger l;

		void applyServerNameDirective(const Directive& directive);
		void applyListenDirective(const Directive& directive);
};
