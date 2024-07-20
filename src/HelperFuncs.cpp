#include <iostream>
#include <iomanip>
#include <ctime>

#include "HelperFuncs.hpp"
#include "Logger.hpp"
#include "HTTPServer.hpp"

// Deprecated, please use the Logger class instead (ga ik wel doen :P)
void log(const std::string& message, const std::string& color) noexcept
{
	// std::cout << color << message << std::endl;
	if (color == Color::Red)
		HTTPServer::instance->l.log(message, L_Error);
	else if (color == Color::Yellow)
		HTTPServer::instance->l.log(message, L_Warning);
	else
		HTTPServer::instance->l.log(message);
}

// Deprecated, please use the Logger class instead
void log(const std::string& message, const LogLevel level) noexcept {
	HTTPServer::instance->l.log(message, level);
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
	size_t pos = 0;
	size_t multiplier = 1;
	int size = std::stoi(str, &pos);
	if (str[pos] == 'K' || str[pos] == 'k')
		multiplier = 1024;
	else if (str[pos] == 'M' || str[pos] == 'm')
		multiplier = 1024 * 1024;
	else if (str[pos] == 'G' || str[pos] == 'g')
		multiplier = 1024 * 1024 * 1024;
	return size * multiplier;
}

std::string strip(const std::string &input, std::string strip_chars)
{
	std::size_t start = input.find_first_not_of(strip_chars);
	std::size_t end = input.find_last_not_of(strip_chars);

	if (start == std::string::npos || end == std::string::npos)
	{
		return "";
	}

	return input.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string &input,
								  const std::string &delimiter,
								  std::size_t max_count)
{
	std::vector<std::string> tokens;
	std::string token;

	size_t pos = 0;
	size_t found;

	while ((found = input.find(delimiter, pos)) != std::string::npos && tokens.size() < max_count)
	{
		token = input.substr(pos, found - pos);
		tokens.push_back(token);
		pos = found + delimiter.size();
	}

	token = input.substr(pos);
	if (!token.empty())
	{
		tokens.push_back(token);
	}

	return tokens;
}

std::string joinPath(std::vector<std::string> paths, std::string delimeter)
{
	std::string joined_path;

	for (size_t i = 0; i < paths.size(); i++)
	{
		std::string stripped = strip(paths[i], "/");
		if (stripped != "")
			joined_path += stripped + delimeter;
	}

	if (paths.back() == "/" || paths.back().back() != '/')
	{
		joined_path.pop_back();
	}
	return joined_path;
}
