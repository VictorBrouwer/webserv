#pragma once

#include <string>
#include <iostream>
#include <unordered_map>

#include "Directive.hpp"

enum LogLevel {
    L_Debug,
    L_Info,
    L_Warning,
    L_Error
};


// Logger class to print messages through. Use the `log()`
// member function to log messages to stdout.
//
// A log message looks like this:
// `[12:59:59][LVL][context] message`
//
// "context" is the `default_context` member variable, unless an
// override is given in the call to the `log()` function.
//
// The log_level member variable indicates from which level messages
// will get printed. This is set by the config, so good practice is
// to inherit it from another logger when you instantiate a new one, like so:
//
// `Logger("new context", original_logger.getLogLevel());`
//
// Or if you will set a different context later, you can use the
// copy constructor like so:
//
// `Logger(original_logger);`
//
// This way, your new logger instance will have the same log level set
// as the original one.
//
//
// What I've done so far is instantiate a new logger only if I need a
// new context in the logs (like "config" instead of "main"). If I don't
// need a new context, I pass a reference to the existing logger and
// call `log()` on that one. This way, the same logger is used in the
// Configuration constructor and in the Directive constructor, and they
// all use the same "config" context.
class Logger {
	public:
		Logger( void );
		Logger(LogLevel level);
		Logger(const std::string& context);
		Logger(const std::string& context, LogLevel level);

		Logger(const Logger& src);

		Logger& operator=(const Logger& src);

		~Logger();

		LogLevel getLogLevel( void ) const;
		const std::string& getDefaultContext() const;

		void setLogLevel(LogLevel level);
		void setLogLevel(const Directive& directive);
		void setDefaultContext(const std::string& context);

		void log(const std::string &message, const LogLevel level = L_Debug) const noexcept;
		void log(const std::string &message, const std::string& context,
				 const LogLevel level = L_Debug) const noexcept;

		static const inline std::unordered_map<std::string, LogLevel> level_map = {
			{"debug",   L_Debug},
			{"info",    L_Info},
			{"warning", L_Warning},
			{"error",   L_Error}
		};

	private:
		std::string default_context = "Default";
		LogLevel    log_level       = L_Debug;
};
