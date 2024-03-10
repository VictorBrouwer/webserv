#include "Directive.hpp"
#include "HelperFuncs.hpp"
#include "Configuration.hpp"
#include "Logger.hpp"
#include "constants.hpp"
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <string>

Directive::Directive(std::vector<std::string>::iterator &i,
					 std::vector<std::string> &lines,
					 const Logger& l)
{
	this->line   = std::distance(lines.begin(), i) + 1;
	this->parent = parent;
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
			stream >> this->key;

			std::string word;
			while (stream >> word)
				this->arguments.push_back(word);

			++i;
			return;
		} else if ((*i)[i->find_last_not_of(WHITESPACE)] == '{') {
			// Multi line directive, construct by splitting and iterating over the block
			l.log("Creating directive with block.");
			std::string line(*i);
			line.pop_back();

			std::istringstream stream(line);
			stream >> this->key;

			std::string word;
			while (stream >> word)
				this->arguments.push_back(word);

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
					this->subdirectives.push_back(Directive(i, lines, l));
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
	return this->key;
}

int Directive::getLine( void ) const {
	return this->line;
}

const std::vector<std::string>& Directive::getArguments( void ) const {
	return this->arguments;
}

const std::vector<Directive>& Directive::getSubdirectives( void ) const {
	return this->subdirectives;
}

std::vector<Directive>::const_iterator Directive::getSubdirectivesIterator( void ) const {
	return this->subdirectives.begin();
}

std::vector<Directive>::const_iterator Directive::getSubdirectivesEnd( void ) const {
	return this->subdirectives.end();
}

std::vector<Directive>::iterator Directive::getSubdirectivesMutableIterator( void ) {
	return this->subdirectives.begin();
}

std::vector<Directive>::iterator Directive::getSubdirectivesMutableEnd( void ) {
	return this->subdirectives.end();
}

void Directive::setParents(Directive* parent) {
	this->parent = parent;
	Directive* current = this;
	std::for_each(
		this->getSubdirectivesMutableIterator(), this->getSubdirectivesMutableEnd(),
		[current](Directive& d) { d.setParents(current); }
	);
}

// Generic validation function

void Directive::validate(const Logger& l, const std::vector<Directive>::const_iterator& context_start,
						 const std::vector<Directive>::const_iterator& context_current) const {
	l.log("Checking if directive \"" + this->key + "\" on line " + std::to_string(this->line) + " is valid");

	// Check whether this directive is recognized by us at all
	if (Configuration::allowed_directives.find(this->key) == Configuration::allowed_directives.end())
		throw Configuration::Exception(E_UNKNOWN_DIRECTIVE, this->line, this->key);

	// Check whether this directive is allowed in its context
	// If parent is a nullptr, our parent is the root context and matches empty string
	const std::string& context = (this->parent ? this->parent->getKey() : "");
	const std::vector<std::string>& allowed = Configuration::allowed_directives.find(context)->second;
	if (std::find(allowed.begin(), allowed.end(), this->key) == allowed.end())
		throw Configuration::Exception(E_WRONG_CONTEXT, this->line, this->key);

	// Check if this is a unique directive and if so, whether we've seen it
	// before in this context
	if (std::find(Configuration::unique_in_context.begin(), Configuration::unique_in_context.end(), this->key)
					!= Configuration::unique_in_context.end()) {
		const std::string& key = this->key;
		if (std::find_if(context_start, context_current, [&](const Directive& d) { return d.getKey() == key; }) != context_current)
			throw Configuration::Exception(E_DUPLICATE_DIRECTIVE, this->line, this->key);
	}

	// Check if this directive has a minimum number of arguments and if so,
	// whether we satisfy it
	if (Configuration::argument_min.find(this->key) != Configuration::argument_min.end()) {
		int size     = this->arguments.size();
		int min_size = Configuration::argument_min.find(this->key)->second;
		if (size < min_size) {
			throw Configuration::Exception("Directive needs at least " +
				std::to_string(min_size) + " arguments, but only had " +
				std::to_string(size), this->line, this->key);
		}
	}

	// Check if this directive has a maximum number of arguments and if so,
	// whether we satisfy it
	if (Configuration::argument_max.find(this->key) != Configuration::argument_max.end()) {
		int size     = this->arguments.size();
		int max_size = Configuration::argument_max.find(this->key)->second;
		if (size < max_size) {
			throw Configuration::Exception("Directive needs at most " +
				std::to_string(max_size) + " arguments, but had " +
				std::to_string(size), this->line, this->key);
		}
	}

	// Check if our directive has a limited list of arguments, and if our
	// arguments satisfy it
	if (Configuration::allowed_arguments.find(this->key) != Configuration::allowed_arguments.end()) {
		const std::vector<std::string>& allowed = Configuration::allowed_arguments.find(this->key)->second;
		std::vector<std::string>::const_iterator it = std::find_if(
			this->arguments.begin(), this->arguments.end(),
			[&](const std::string& s) { return std::find(allowed.begin(), allowed.end(), s) == allowed.end(); }
		);
		if (it != this->arguments.end()) {
			throw Configuration::Exception(
				"Argument \"" + *it + "\" is invalid for directive, allowed arguments are: " +
				vector_to_string(allowed), this->line, this->key
			);
		}
	}

	// Check if our directive has a custom validator registered, and if so, run it
	// It is the validator function's job to raise an exception if
	// something is wrong.
	if (Configuration::arg_validators.find(this->key) != Configuration::arg_validators.end()) {
		(this->*Configuration::arg_validators.find(this->key)->second)(l);
	}

	// Recurse onto subdirectives
	std::vector<Directive>::const_iterator start = this->getSubdirectivesIterator();
	std::vector<Directive>::const_iterator end   = this->getSubdirectivesEnd();
	std::vector<Directive>::const_iterator it    = start;

	while (it != end) {
		it->validate(l, start, it);
		++it;
	}
}

// Directive-specific validators

void Directive::client_max_body_size_validator(const Logger& l) const {
	l.log("Custom directive checker is not implemented yet!", L_Warning);
}

void Directive::error_page_validator(const Logger& l) const {
	l.log("Custom directive checker is not implemented yet!", L_Warning);
}

void Directive::return_validator(const Logger& l) const {
	l.log("Custom directive checker is not implemented yet!", L_Warning);
}
