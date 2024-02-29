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

	private:
		std::vector<Directive> _directives;
};
