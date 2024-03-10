#pragma once
#include <string>
#include <iostream>
#include <map>
#include "Directive.hpp"

enum LogLevel {
    L_Debug,
    L_Info,
    L_Warning,
    L_Error
};

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

		void setLogLevel(const LogLevel level);
		void setLogLevel(const Directive& directive);
		void setDefaultContext(const std::string& context);

		void log(const std::string &message, const LogLevel level = L_Debug) const noexcept;
		void log(const std::string &message, const std::string& context,
				 const LogLevel level = L_Debug) const noexcept;

		static const inline std::map<std::string, LogLevel> level_map = {
			{"debug",   L_Debug},
			{"info",    L_Info},
			{"warning", L_Warning},
			{"error",   L_Error}
		};

	private:
		std::string default_context = "Default";
		LogLevel    log_level       = L_Debug;
};
