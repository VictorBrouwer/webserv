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
		config_path = "./webserv.conf";
	else
		config_path = av[1];
	log("Using config file: " + config_path);

	std::ifstream config_file(config_path);
	if (!config_file.is_open())
		exitWithError("Could not open config file, exiting.");

	return 0;
}
