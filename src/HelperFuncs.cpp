#include "HelperFuncs.hpp"
#include "Logger.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>


// Deprecated, please use the Logger class instead
void log(const std::string& message, const std::string& color) noexcept
{
	Logger l;

	// std::cout << color << message << std::endl;
	if (color == Color::Red)
		l.log(message, L_Error);
	else if (color == Color::Yellow)
		l.log(message, L_Warning);
	else
		l.log(message);
}

// Deprecated, please use the Logger class instead
void log(const std::string& message, const LogLevel level) noexcept {
	Logger l;
	l.log(message, level);
}

// Deprecated, please use the Logger class instead
void exitWithError(const std::string &errorMessage)
{
	log("ERROR: " + errorMessage, Color::Red);
	exit(1);
}
