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

	private:
		std::vector<Directive> _directives;
};
