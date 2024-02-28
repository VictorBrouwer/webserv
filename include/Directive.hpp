#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class Directive {
	public:
		Directive(std::stringstream content);
		~Directive();

	private:
		std::string              m_key;
		std::string              m_value;
		std::vector<std::string> m_options;
		std::vector<Directive>   m_directives;
};
