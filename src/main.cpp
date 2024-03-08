#include"HTTPServer.hpp"
#include "HelperFuncs.hpp"
#include "Directive.hpp"
#include "constants.hpp"
#include "Logger.hpp"
#include <iostream>
#include <fstream>
#include <memory>

// int main()
// {
// 	HTTPServer HTTPServer("0.0.0.0", 8080);
// 	HTTPServer.startListen();
// 	HTTPServer.startPolling();

// 	return 0;
// }

int main(int ac, char **av)
{
	std::string config_path;
	Logger      l("main", L_Info);

	l.log("Starting webserv! 🌐", L_Info);

	if (ac == 1)
		config_path = "./config/webserv.conf";
	else
		config_path = av[1];
	l.log("Using config file: " + config_path, L_Info);

	std::ifstream config_file(config_path);
	if (!config_file.is_open()) {
		l.log("Could not open config file, exiting.", L_Error);
		return 1;
	}

	int return_value = 0;
	try {
		std::unique_ptr<Configuration> config( new Configuration(config_file) );
		config->validate();
		l.log("Config is all good!", L_Info);
		std::unique_ptr<HTTPServer> http_server( new HTTPServer(*config) );
		// 	HTTPServer->startListen();
		// 	HTTPServer->startPolling();
	}
	catch(const std::exception& e) {
		l.log(e.what(), L_Error);
		l.log("Cannot recover, exiting.", L_Error);
		return_value = 1;
	}

	return return_value;
}
