#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class Directive {
	public:
		Directive(std::vector<std::string>::iterator &iterator,
				  std::vector<std::string> &lines);
		~Directive();

		const std::string&              getKey( void ) const;
		int                             getLine( void ) const;
		const std::vector<std::string>& getArguments( void ) const;
		const std::vector<Directive>&   getBlock( void ) const;

	private:
		std::string              _key;
		int                      _line;
		std::vector<std::string> _arguments;
		std::vector<Directive>   _block;

	// Directive exceptions are thrown when validating the configuration,
	// if a directive is not recognized, duplicated, has incorrect arguments or
	// is not allowed to be used in its scope.
	public:
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
