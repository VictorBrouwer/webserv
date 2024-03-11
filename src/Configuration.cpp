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

	this->l = Logger("config", logger.getLogLevel());

	try {
		l.log("Reading config file.");
		while ( std::getline(config_file, line) )
			config_lines.push_back(line);
		l.log("Done reading after " + std::to_string(config_lines.size()) + " lines.");

		if (config_lines.empty())
			throw Configuration::Exception("No lines read from config file");

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
				l.log("Skipping comment or empty line.");
				++i;
			}
			else {
				// The constructor should take care of moving our iterator forward.
				this->directives.push_back(Directive(i, config_lines, l));
			}
		}

		l.log("Setting parents of directives.");
		std::for_each(
			this->getDirectiveMutableIterator(), this->getDirectiveMutableEnd(),
			[](Directive& d) { d.setParents(nullptr); }
		);
	}
	catch(const Configuration::Exception& e) {
		l.log(e.what(), L_Error);
		// Rethrow to catch it again in the main function
		throw;
	}
}

Configuration::~Configuration( void ) {

}

std::vector<Directive>::const_iterator Configuration::getDirectiveIterator( void ) const {
	return this->directives.begin();
}

std::vector<Directive>::const_iterator Configuration::getDirectiveEnd( void ) const {
	return this->directives.end();
}

std::vector<Directive>::iterator Configuration::getDirectiveMutableIterator( void ) {
	return this->directives.begin();
}

std::vector<Directive>::iterator Configuration::getDirectiveMutableEnd( void ) {
	return this->directives.end();
}

// Validates the configuration, throwing a
// Configuration::Exception if an error is raised.
void Configuration::validate(const Logger& logger) {
	l.setLogLevel(logger.getLogLevel());
	l.log("Validating config.", L_Info);
	try {
		std::vector<Directive>::const_iterator start = this->getDirectiveIterator();
		std::vector<Directive>::const_iterator end   = this->getDirectiveEnd();

		l.log("Checking for http directive");
		if (std::find_if(start, end, [](const Directive& d) { return d.getKey() == "http"; }) == end)
			throw Configuration::Exception(E_MISSING_DIRECTIVE, "http");

		l.log("Validating directives");
		std::vector<Directive>::const_iterator i = start;
		while (i != end) {
			i->validate(l, start, i);
			++i;
		}

		l.log("Config is all good!", L_Info);
	}
	catch(const std::exception& e) {
		l.log(std::string("Config validation failed: ") + e.what(), L_Error);
		throw;
	}


	return;
}
