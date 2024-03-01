#include "Configuration.hpp"
#include "HelperFuncs.hpp"
#include "constants.hpp"
#include <iostream>
#include <string>

Configuration::Configuration(std::ifstream &config_file) {
	std::string		         line;
	std::vector<std::string> config_lines;

	log("Reading config file.");
	while ( std::getline(config_file, line) )
		config_lines.push_back(line);
	log("Done reading after " + std::to_string(config_lines.size()) + " lines.");

	if (config_lines.empty())
		throw std::invalid_argument("No lines read from config file");

	// Remove comments from our string vector
	std::vector<std::string>::iterator i = config_lines.begin();
	while (i != config_lines.end())
	{
		if ((*i).find('#') != std::string::npos)
			(*i).erase((*i).find('#'));
		++i;
	}

	// Pass the iterator to consecutive constructors. Because it is a
	// reference, the underlying iterator moves along and picks up where
	// we left off when we "unscope" from one directive back to the parent.
	i = config_lines.begin();
	while (i != config_lines.end()) {
		if (i->find_first_not_of(WHITESPACE) == std::string::npos || i->empty()) {
			log("Skipping comment or empty line.");
			++i;
		}
		else {
			// The constructor should take care of moving our iterator forward.
			this->_directives.push_back(Directive(i, config_lines));
		}
	}
}

Configuration::~Configuration( void ) { }
