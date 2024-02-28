#include"HTTPserver.hpp"
#include "HelperFuncs.hpp"
#include <iostream>
#include <fstream>

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

	log("Starting webserv! 🌐");

	if (ac == 1)
		config_path = "./config/webserv.conf";
	else
		config_path = av[1];
	log("Using config file: " + config_path);

	std::ifstream config_file(config_path);
	if (!config_file.is_open())
		exitWithError("Could not open config file, exiting.");

	HTTPServer    *http_server;
	try
	{
		Configuration configuration(config_file);
		http_server = new HTTPServer(configuration);
		// 	HTTPServer->startListen();
		// 	HTTPServer->startPolling();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		delete http_server;
	}

	return 0;
}
