#include "Directive.hpp"
#include "HelperFuncs.hpp"
#include "Configuration.hpp"
#include "Logger.hpp"
#include "constants.hpp"
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <iostream>

Directive::Directive(std::vector<std::string>::iterator &i,
					 std::vector<std::string> &lines,
					 const Logger& l, Directive *parent)
{
	this->_line   = std::distance(lines.begin(), i);
	this->_parent = parent;
	while (i != lines.end())
	{
		if (i->find_first_not_of(WHITESPACE) == std::string::npos || i->empty()) {
			// Skip empty lines and comments
			l.log("Skipping comment or empty line.");
			++i;
		} else if ((*i)[i->find_last_not_of(WHITESPACE)] == ';') {
			// Single line directive, construct by splitting
			l.log("Creating blockless directive.");
			std::string line(*i);
			line.pop_back();

			std::istringstream stream(line);
			stream >> this->_key;

			std::string word;
			while (stream >> word)
				this->_arguments.push_back(word);

			++i;
			return;
		} else if ((*i)[i->find_last_not_of(WHITESPACE)] == '{') {
			// Multi line directive, construct by splitting and iterating over the block
			l.log("Creating directive with block.");
			std::string line(*i);
			line.pop_back();

			std::istringstream stream(line);
			stream >> this->_key;

			std::string word;
			while (stream >> word)
				this->_arguments.push_back(word);

			// Subloop for nested directives. Skip empty lines and comments
			// again, create directives for other things, stop when encountering
			// the closing bracket. If we hit the end of the vector before then,
			// throw a syntax error.
			std::vector<std::string>::iterator block_start(i);
			++i;
			while (i != lines.end() && (*i)[i->find_first_not_of(WHITESPACE)] != '}')
			{
				if (i->find_first_not_of(WHITESPACE) == std::string::npos || i->empty()) {
					l.log("Skipping comment or empty line.");
					++i;
				} else {
					this->_block.push_back(Directive(i, lines, l, this));
				}
			}
			if (i == lines.end()) {
				throw Configuration::Exception(E_BLOCK_NOT_CLOSED, std::distance(lines.begin(), i) + 1, *block_start);
			}
			l.log("Directive block ends.");
			++i;
			return;
		} else {
			// Something unexpected, we throw out
			throw Configuration::Exception(E_INCORRECT_FORMAT, std::distance(lines.begin(), i) + 1, *i);
		}
	}
}

Directive::~Directive( void ) { }

const std::string& Directive::getKey( void ) const {
	return this->_key;
}

int Directive::getLine( void ) const {
	return this->_line;
}

const std::vector<std::string>& Directive::getArguments( void ) const {
	return this->_arguments;
}

const std::vector<Directive>& Directive::getBlock( void ) const {
	return this->_block;
}
