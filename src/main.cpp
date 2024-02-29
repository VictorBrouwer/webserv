#include"HTTPServer.hpp"
#include "HelperFuncs.hpp"
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

	log("Starting webserv! 🌐");

	if (ac == 1)
		config_path = "./config/webserv.conf";
	else
		config_path = av[1];
	log("Using config file: " + config_path);

	std::ifstream config_file(config_path);
	if (!config_file.is_open())
		exitWithError("Could not open config file, exiting.");

	int return_value = 0;
	try
	{
		std::unique_ptr<Configuration> config(new Configuration(config_file) );
		// std::unique_ptr<HTTPServer>    http_server(new HTTPServer(*config) );
		// 	HTTPServer->startListen();
		// 	HTTPServer->startPolling();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return_value = 1;
	}

	return return_value;
}
