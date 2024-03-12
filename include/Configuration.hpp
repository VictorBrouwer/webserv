#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "Directive.hpp"
#include "Logger.hpp"

class Configuration {
	public:
		Configuration(std::ifstream &config_file, const Logger& logger);
		~Configuration();

		const Directive& getHttpDirective( void ) const;

		std::vector<Directive>::const_iterator getDirectiveIterator( void ) const;
		std::vector<Directive>::const_iterator getDirectiveEnd( void ) const;

		std::vector<Directive>::iterator getDirectiveMutableIterator( void );
		std::vector<Directive>::iterator getDirectiveMutableEnd( void );

		void validate(const Logger& logger);

	private:
		std::vector<Directive> directives;
		Logger l;

	// Additional stuff we need in the configuration
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

		// Map of directive strings with a vector of allowed subdirectives under
		// it. Empty string means root directive. This is also an exhaustive list
		// of directives, so if a directive string is not in this map, it is
		// not recognized by us and should throw an error.
		static const inline std::unordered_map<std::string, std::vector<std::string>> allowed_directives = {
			{"",                     {"http", "log_level"}},
			{"autoindex",            {}},
			{"client_max_body_size", {}},
			{"error_page",           {}},
			{"http",                 {"autoindex", "client_max_body_size",
									  "error_page", "index", "root", "server"}},
			{"index",                {}},
			{"limit_except",         {}},
			{"listen",               {}},
			{"location",             {"autoindex", "client_max_body_size",
									  "error_page", "index", "limit_except",
									  "location", "return", "root"}},
			{"log_level",            {}},
			{"return",               {}},
			{"root",                 {}},
			{"server",               {"autoindex", "client_max_body_size",
									  "error_page", "index", "listen", "location",
									  "return", "root", "server_name"}},
			{"server_name",          {}},
		};

		// Vector of directives that cannot have more than one
		// in the given context. The second one will throw an exception.
		static const inline std::vector<std::string> unique_in_context = {
			"autoindex",
			"client_max_body_size",
			"http",
			"index",
			"log_level",
			"return",
			"root"
		};

		// Map of directive strings with an integer if the directive has a
		// minimum number of arguments.
		static const inline std::unordered_map<std::string, int> argument_min = {
			{"autoindex",            1},
			{"client_max_body_size", 1},
			{"error_page",           2},
			{"index",                1},
			{"limit_except",         1},
			{"listen",               1},
			{"location",             1},
			{"log_level",            1},
			{"return",               1},
			{"root",                 1},
			{"server_name",          1}
		};

		// Map of directive strings with an integer if the directive has a
		// maximum number of arguments.
		static const inline std::unordered_map<std::string, int> argument_max = {
			{"autoindex",            1},
			{"client_max_body_size", 1},
			{"http",                 0},
			{"log_level",            1},
			{"return",               2},
			{"root",                 1},
			{"server",               0}
		};

		// Map of directive strings with a vector describing its allowed arguments
		// If a directive is not in this map, it allows arbitrary arguments
		static const inline std::unordered_map<std::string, std::vector<std::string>> allowed_arguments = {
			{"autoindex",    {"on", "off"},                        },
			{"limit_except", {"GET", "POST", "DELETE"},            },
			{"log_level",    {"debug", "info", "warning", "error"} },
		};

		// Map of directive strings with a member function pointer for a
		// validator to call on them, in case their arguments have requirements
		// not covered by the standard checks performed in here.
		//
		// Value has to be a pointer to a const member function of the Directive
		// class that takes a const reference to the config logger and
		// returns nothing. This function is expected to throw a
		// Configuration::Exception if its arguments are invalid.
		typedef void (Directive::*validator)( const Logger& ) const;
		static const inline std::unordered_map<std::string, validator> arg_validators = {
			{"client_max_body_size", &Directive::client_max_body_size_validator },
			{"error_page",           &Directive::error_page_validator },
			{"return",               &Directive::return_validator },
		};

};
