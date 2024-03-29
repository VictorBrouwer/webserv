#include "Logger.hpp"
#include "HelperFuncs.hpp"

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <algorithm>
#include <ctime>

// Constructors

Logger::Logger( void ) { }

Logger::Logger(LogLevel level) {
	this->log_level = level;
}

Logger::Logger(const std::string& context, LogLevel level) : default_context(context) {
	this->log_level = level;
}

Logger::Logger(const std::string& context) : default_context(context) { }

Logger::Logger(const Logger& src) {
	*this = src;
}

// Operator overloads

Logger& Logger::operator=(const Logger& src) {
	this->default_context = src.default_context;
	this->log_level       = src.log_level;

	return *this;
}

// Destructor

Logger::~Logger() { }

// Getters

LogLevel Logger::getLogLevel() const {
	return this->log_level;
}

const std::string& Logger::getDefaultContext() const {
	return this->default_context;
}

// Setters

void Logger::setLogLevel(LogLevel level) {
	this->log_level = level;
}

void Logger::setLogLevel(const Directive& directive) {
	std::vector<std::string> args = directive.getArguments();
	if (Logger::level_map.find(args[0]) != Logger::level_map.end()) {
		this->log("Setting log level to " + args[0], L_Info);
		this->log_level = Logger::level_map.at(args[0]);
	} else {
		this->log_level = L_Info;
		this->log("Unrecognized log level, defaulting to info", L_Warning);
	}
}

void Logger::setDefaultContext(const std::string& context) {
	this->default_context = context;
}

// Member functions

void Logger::log(const std::string& message, const LogLevel level) const noexcept {
	this->log(message, this->default_context, level);
}

void Logger::log(const std::string& message, const std::string& context, const LogLevel level) const noexcept {
	// If this log call wants to print something we do not care about
	// at our current log level, we can just return.
	if (this->log_level > level)
		return;

	time_t	time = std::time(nullptr);
	tm		tm   = *std::localtime(&time);
	std::cout << std::put_time(&tm, "[%H:%M:%S]");

	switch (level) {
	case L_Debug:
		std::cout << Color::Reset << "[DBG]";
		break;
	case L_Info:
		std::cout << Color::Cyan << "[INF]";
		break;
	case L_Warning:
		std::cout << Color::Yellow << "[WRN]";
		break;
	case L_Error:
		std::cout << Color::Red << "[ERR]";
		break;
	}

	std::cout << "[" << context << "]";
	std::cout << " " << message << Color::Reset << std::endl;
}
