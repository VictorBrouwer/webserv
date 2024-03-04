#include "HelperFuncs.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>

// Function definitions
void log(const std::string& message, const std::string& color) noexcept
{
	// std::cout << color << message << std::endl;
	if (color == Color::Red)
		log(message, L_Error);
	else if (color == Color::Yellow)
		log(message, L_Warning);
	else
		log(message);
}

void log(const std::string& message, const LogLevel level) noexcept {
	time_t	time = std::time(nullptr);
	tm		tm   = *std::localtime(&time);
	std::cout << std::put_time(&tm, "[%H:%M:%S]");

	switch (level)
	{
	case L_Debug:
		std::cout << Color::Reset << "[DEBUG]";
		break;
	case L_Info:
		std::cout << Color::Cyan << "[INFO ]";
		break;
	case L_Warning:
		std::cout << Color::Yellow << "[WARN ]";
		break;
	case L_Error:
		std::cout << Color::Red << "[ERROR]";
		break;
	default:
		break;
	}

	std::cout << " " << message << Color::Reset << std::endl;
}

void exitWithError(const std::string &errorMessage)
{
	log("ERROR: " + errorMessage, Color::Red);
	exit(1);
}
