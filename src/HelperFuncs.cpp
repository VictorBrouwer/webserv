#include "HelperFuncs.hpp"

// Function definitions
void log(const std::string& message, const std::string& color)
{
	std::cout << color << message << std::endl;
}

void exitWithError(const std::string &errorMessage)
{
	log("ERROR: " + errorMessage, Color::Red);
	exit(1);
}
