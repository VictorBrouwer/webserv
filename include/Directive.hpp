#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class Logger;

class Directive {
	public:
		Directive(std::vector<std::string>::iterator &iterator,
				  std::vector<std::string> &lines,
				  const Logger& logger, Directive *parent = nullptr);
		~Directive();

		const std::string&              getKey( void ) const;
		int                             getLine( void ) const;
		const std::vector<std::string>& getArguments( void ) const;
		const std::vector<Directive>&   getBlock( void ) const;

	private:
		std::string              _key;
		int                      _line;
		std::vector<std::string> _arguments;
		std::vector<Directive>   _block;
		Directive*               _parent;
};
