#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class Directive {
	public:
		Directive(std::vector<std::string>::iterator &iterator,
				  const std::vector<std::string> &lines);
		~Directive();

	private:
		std::string              _key;
		std::vector<std::string> _arguments;
		std::vector<Directive>   _block;
};