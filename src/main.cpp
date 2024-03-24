#include "HTTPServer.hpp"
#include "HelperFuncs.hpp"
#include "Directive.hpp"
#include "constants.hpp"
#include "Logger.hpp"
#include <iostream>
#include <fstream>
#include <memory>

// int main() {
// 	HTTPServer HTTPServer("0.0.0.0", 8080);
// 	HTTPServer.startListen();
// 	HTTPServer.startPolling();

// 	return 0;
// }

int main(int ac, char **av)
{
	std::string config_path;
	// Before we've parsed the config, we don't know what the log level
	// should be. This is the setting for the level until then.
	// Change me to L_Debug for debug messages from the config parser
	Logger      l("main", L_Info);

	l.log("Starting webserv! 🌐", L_Info);

	if (ac == 1)
		config_path = "/home/jmeruma/Documents/personal/webserv/config/webserv.conf";
	else
		config_path = av[1];
	l.log("Using config file: " + config_path, L_Info);

	int return_value = 0;
	try {
		std::ifstream config_file(config_path);
		if (!config_file.is_open()) {
			throw(Configuration::Exception("Could not open config file"));
		}

		std::unique_ptr<Configuration> config( new Configuration(config_file, l) );
		config_file.close();

		l.log("Config loaded successfully.", L_Info);

		if (std::find(config->getDirectiveIterator(), config->getDirectiveEnd(), "log_level") != config->getDirectiveEnd()) {
			l.setLogLevel(*(std::find(config->getDirectiveIterator(), config->getDirectiveEnd(), "log_level")));
		} else {
			l.log("No log level set, defaulting to info.", L_Info);
		}

		config->validate(l);

		std::unique_ptr<HTTPServer> http_server( new HTTPServer(*config, l) );
		http_server->startListening();
		http_server->startPolling();
	}
	catch(const std::exception& e) {
		l.log(std::string("Uncaught or unrecoverable exception thrown: ") + e.what(), L_Error);
		l.log("Cannot continue, exiting.", L_Error);

		return_value = 1;
	}

	return return_value;
}
