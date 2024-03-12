#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <iterator>
#include <string>

class Logger;

class Directive {
	public:
		Directive(std::vector<std::string>::iterator &iterator,
				  std::vector<std::string> &lines,
				  const Logger& logger);
		~Directive();

		const std::string&                     getKey( void ) const;
		int                                    getLine( void ) const;
		const std::vector<std::string>&        getArguments( void ) const;
		const std::vector<Directive>&          getSubdirectives( void ) const;

		std::vector<std::string>::const_iterator getArgumentsIterator( void ) const;
		std::vector<std::string>::const_iterator getArgumentsEnd( void ) const;

		std::vector<Directive>::const_iterator getSubdirectivesIterator( void ) const;
		std::vector<Directive>::const_iterator getSubdirectivesEnd( void ) const;

		std::vector<Directive>::iterator getSubdirectivesMutableIterator( void );
		std::vector<Directive>::iterator getSubdirectivesMutableEnd( void );

		bool operator==(const std::string& str) const;

		void setParents(Directive* parent);

		void validate(const Logger& l,
					  const std::vector<Directive>::const_iterator& context_start,
					  const std::vector<Directive>::const_iterator& context_current) const;

		// Argument validators for specific directives
		void client_max_body_size_validator(const Logger& l) const;
		void error_page_validator(const Logger& l) const;
		void return_validator(const Logger& l) const;

	private:
		std::string              key;
		int                      line;
		std::vector<std::string> arguments;
		std::vector<Directive>   subdirectives;
		Directive*               parent;

};
