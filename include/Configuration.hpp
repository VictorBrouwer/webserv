#pragma once
#include "Directive.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class Configuration {
	public:
		Configuration(std::ifstream &config_file);
		~Configuration();

		std::vector<Directive>::iterator getDirectoryIterator( void );
		std::vector<Directive>::iterator getDirectoryEnd( void );

		bool validate( void );

	private:
		std::vector<Directive> _directives;

	public:

		// Configuration exceptions are thrown during parsing, when a syntax
		// error prevents us from continuing.
		class Exception : public std::exception {
			protected:
				const std::string _message;

			public:
				Exception(const std::string& reason, int line, const std::string& context) :
				_message("Config error on line " + std::to_string(line) + " : "
												+ reason + " ( " + context + " )") { }

				Exception(const std::string& reason, int line) :
				_message("Config error on line " + std::to_string(line) + " : " + reason) { }

				virtual const char* what() const throw() {
					return _message.c_str();
				}
		};
};
