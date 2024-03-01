#include "Directive.hpp"
#include "HelperFuncs.hpp"
#include "constants.hpp"
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <iostream>

Directive::Directive(std::vector<std::string>::iterator &i,
					 const std::vector<std::string> &lines)
{
	while (i != lines.end())
	{
		if (i->find_first_not_of(WHITESPACE) == std::string::npos || i->empty()) {
			// Skip empty lines and comments
			log("Skipping comment or empty line.");
			++i;
		} else if ((*i)[i->find_last_not_of(WHITESPACE)] == ';') {
			// Single line directive, construct by splitting
			log("Creating blockless directive.");
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
			log("Creating directive with block.");
			std::string line(*i);
			line.pop_back();

			std::istringstream stream(line);
			stream >> this->_key;

			std::string word;
			while (stream >> word)
				this->_arguments.push_back(word);

			++i;
			// Subloop for nested directives. Skip empty lines and comments
			// again, create directives for other things, stop when encountering
			// the closing bracket. If we hit the end of the vector before then,
			// throw a syntax error.
			while (i != lines.end() && (*i)[i->find_first_not_of(WHITESPACE)] != '}')
			{
				if (i->find_first_not_of(WHITESPACE) == std::string::npos || i->empty()) {
					log("Skipping comment or empty line.");
					++i;
				} else {
					this->_block.push_back(Directive(i, lines));
				}
			}
			if (i == lines.end()) {
				throw std::invalid_argument("Directive block not properly closed");
			}
			log("Directive block ends.");
			++i;
			return;
		} else {
			// Something unexpected, we throw out
			throw std::invalid_argument("Incorrectly formatted config line");
		}
	}
}

Directive::~Directive( void ) { }
