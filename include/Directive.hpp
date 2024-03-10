#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class Logger;

class Directive {
	public:
		Directive(std::vector<std::string>::iterator &iterator,
				  std::vector<std::string> &lines,
				  const Logger& logger,
				  Directive *parent = nullptr);
		~Directive();

		const std::string&              getKey( void ) const;
		int                             getLine( void ) const;
		const std::vector<std::string>& getArguments( void ) const;
		const std::vector<Directive>&   getBlock( void ) const;

		void validate(const Logger& l,
					  const std::vector<Directive>::iterator& start,
					  const std::vector<Directive>::iterator& current) const;

		// Argument validators for specific directives
		void client_max_body_size_validator(const Logger& l) const;
		void error_page_validator(const Logger& l) const;
		void return_validator(const Logger& l) const;

	private:
		std::string              key;
		int                      line;
		std::vector<std::string> arguments;
		std::vector<Directive>   block;
		Directive*               parent;

};
