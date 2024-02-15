#include "HelperFuncs.hpp"

// Function definitions
void log(const std::string &color, const std::string &message)
{
	std::cout << color << message << std::endl;
}

void exitWithError(const std::string &errorMessage)
{
	log(Color::Red, "ERROR: " + errorMessage);
	exit(1);
}