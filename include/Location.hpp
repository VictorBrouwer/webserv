#pragma once

#include <vector>
#include <string>

#include "Logger.hpp"
#include "Directive.hpp"
#include "ConfigShared.hpp"
#include "ConfigReturn.hpp"
#include "ClientState.hpp"

class Location : public ConfigShared, public ConfigReturn {
	public:
		Location(const Directive& directive, ConfigShared* shared, const Logger& logger);

		bool operator==(const std::string& uri) const;
		std::string & getUri();
		bool checkMethod(HTTPMethod method);
		bool getReturnActive();
		int getReturnStatusCode();
		std::string& getReturnBody();

		~Location();

	private:
		Logger l;

		std::string              uri;
		std::vector<std::string> allowed_methods = {"GET", "POST", "DELETE"};
		std::vector<Location>    locations;

};
