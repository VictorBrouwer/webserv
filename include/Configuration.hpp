#pragma once
#include "Directive.hpp"
#include "Logger.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class Configuration {
	public:
		Configuration(std::ifstream &config_file, const Logger& logger);
		~Configuration();

		std::vector<Directive>::iterator getDirectoryIterator( void );
		std::vector<Directive>::iterator getDirectoryEnd( void );

		bool validate( void );

	private:
		std::vector<Directive> _directives;
		Logger                 _logger;

	public:

		// Configuration exceptions are parsing errors, invalid syntax or
		// directives that will not allow us to continue.
		class Exception : public std::exception {
			private:
				const std::string _message;

			public:
				Exception(const std::string& reason, int line, const std::string& context) :
				_message("Config error on line " + std::to_string(line) + " : "
												+ reason + " ( " + context + " )") { }

				Exception(const std::string& reason, int line) :
				_message("Config error on line " + std::to_string(line) + " : " + reason) { }

				Exception(const std::string& reason) :
				_message("Config error : " + reason) { }

				Exception(const std::string& reason, const std::string& context) :
				_message("Config error : " + reason + " ( " + context + " )") { }

				virtual const char* what() const throw() {
					return _message.c_str();
				}
		};
};
