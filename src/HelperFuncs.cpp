#include <iostream>
#include <iomanip>
#include <ctime>

#include "HelperFuncs.hpp"
#include "Logger.hpp"

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

// Converts a vector of strings to "[s1,s2,s3]"
std::string vector_to_string(const std::vector<std::string>& vector) {
	std::string result = "[";

	std::for_each(vector.begin(), vector.end(), [&](const std::string& s) {
		result += s;
		result += ",";
	});

	result.pop_back();
	result += "]";

	return result;
}

size_t size_to_int(const std::string& str) {
	log("size_to_int not implemented yet", L_Warning);
	(void) str;
	return 0;
}
