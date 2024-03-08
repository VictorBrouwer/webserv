#include "Configuration.hpp"
#include "HelperFuncs.hpp"
#include "Directive.hpp"
#include "constants.hpp"
#include "Logger.hpp"
#include <iostream>
#include <string>
#include <algorithm>

Configuration::Configuration(std::ifstream &config_file, const Logger& logger) {
	std::string		         line;
	std::vector<std::string> config_lines;

	this->_logger = Logger("config", logger.getLogLevel());

	try {
		_logger.log("Reading config file.", L_Info);
		while ( std::getline(config_file, line) )
			config_lines.push_back(line);
		_logger.log("Done reading after " + std::to_string(config_lines.size()) + " lines.", L_Info);

		if (config_lines.empty())
			throw Configuration::Exception("No lines read from config file", 1);

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
				_logger.log("Skipping comment or empty line.");
				++i;
			}
			else {
				// The constructor should take care of moving our iterator forward.
				this->_directives.push_back(Directive(i, config_lines, _logger));
			}
		}
	}
	catch(const Configuration::Exception& e) {
		_logger.log(e.what(), L_Error);
		// Rethrow to catch it again in the main function
		throw;
	}
}

Configuration::~Configuration( void ) {

}

std::vector<Directive>::iterator Configuration::getDirectoryIterator( void ) {
	return this->_directives.begin();
}

std::vector<Directive>::iterator Configuration::getDirectoryEnd( void ) {
	return this->_directives.end();
}

// Validates the configuration, throwing a
// Configuration::Exception if an error is raised.
bool Configuration::validate( void ) {
	std::vector<Directive>::iterator i   = this->getDirectoryIterator();
	std::vector<Directive>::iterator end = this->getDirectoryEnd();

	_logger.log("Checking for http directive");
	int http_count      = std::count_if(i, end, [](const Directive& d) { return d.getKey() == "http"; });
	// int log_level_count = std::count_if(i, end, [](const Directive& d) { return d.getKey() == "log_level"; });

	switch (http_count)
	{
	case 0:
		throw Configuration::Exception(E_MISSING_DIRECTIVE, "http");
		break;
	case 1:
		break;
	default:
		throw Configuration::Exception(E_DUPLICATE_DIRECTIVE, "http");
		break;
	}

	return true;
}
