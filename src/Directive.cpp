#include "Directive.hpp"
#include "HelperFuncs.hpp"
#include "Configuration.hpp"
#include "Logger.hpp"
#include "constants.hpp"

Directive::Directive(std::vector<std::string>::iterator &i,
					 std::vector<std::string> &lines,
					 const Logger& l)
{
	this->line   = std::distance(lines.begin(), i) + 1;
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

std::vector<std::string>::const_iterator Directive::getArgumentsIterator( void ) const {
	return this->arguments.begin();
}

std::vector<std::string>::const_iterator Directive::getArgumentsEnd( void ) const {
	return this->arguments.end();
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

// Throws std::invalid_argument if there is no server_name directive under
// this one.
const std::string& Directive::getPrimaryServerName( void ) const {
	auto start = this->getSubdirectivesIterator();
	auto end   = this->getSubdirectivesEnd();

	auto it    = std::find(start, end, "server_name");
	if (it == end)
		throw std::invalid_argument("No server_name directive under this one");
	return it->getArguments()[0];
}

bool Directive::operator==(const std::string& str) const {
	return this->getKey() == str;
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
		if (std::find(context_start, context_current, key) != context_current) {
			// throw Configuration::Exception(E_DUPLICATE_DIRECTIVE, this->line, this->key);
			l.log("Directive \"" + this->key + "\" on line " +
				std::to_string(this->line) + " already exists in this context and will be ignored.",
				L_Warning
			);
		}
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
		if (size > max_size) {
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
				"Argument \"" + *it + "\" is invalid for directive \"" + this->key +
				"\", allowed arguments are: " + vector_to_string(allowed), this->line
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
	if (arguments.size() != 1)
		throw Configuration::Exception("Too many arguments for client max body size", line);
	if (key != "client_max_body_size")
		throw Configuration::Exception("Client max body size key is incorrect", line);
	std::string arg = arguments[0];
	for (char c : arg)
	{
		if (!std::isdigit(c) && std::tolower(c) != 'k' && std::tolower(c) != 'm' && std::tolower(c) != 'g')
			throw Configuration::Exception("Invalid client max body size", line);
	}
	size_t pos = 0;
	try
	{
		int size = std::stoi(arg, &pos);
		// Check if the entire string was parsed
		if (pos == arg.size())
			return;
		// Check if size is non-negative
		if (size < 0)
			throw Configuration::Exception("Invalid client max body size", line);
		// Check if string ends with only 1 letter
		if (arg[pos] != arg[arg.size() - 1])
			throw Configuration::Exception("Invalid client max body size", line);
		// Check for optional suffixes (K, M, G)
		if (arg[pos] == 'K' || arg[pos] == 'k' || 
			arg[pos] == 'M' || arg[pos] == 'm' || 
			arg[pos] == 'G' || arg[pos] == 'g')
			return;
		if (pos != arg.size())
			throw Configuration::Exception("Invalid client max body size", line);
		// If no suffix is present, size is specified in bytes
	}
	// stoi throws invalid_argument if the string is not a valid integer
	catch (const std::invalid_argument& e)
	{
		throw Configuration::Exception("Client max body size argument is not a valid integer", line);
	}
	// stoi throws out_of_range if the converted value is out of range
    catch (const std::out_of_range& e)
	{
		throw Configuration::Exception("Client max body size argument is out of range", line);
	}
	l.log("Custom directive checker \"client_max_body_size\" is not implemented yet!", L_Warning);
}

void Directive::error_page_validator(const Logger& l) const {
	l.log("Custom directive checker \"error_page\" is not implemented yet!", L_Warning);
}

void Directive::return_validator(const Logger& l) const {
	l.log("Custom directive checker \"return\" is not implemented yet!", L_Warning);
}
