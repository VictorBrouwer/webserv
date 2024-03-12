#include "Configurable.hpp"
#include "HelperFuncs.hpp"

Configurable::Configurable(const Configurable& src) {
	this->autoindex_enabled    = src.autoindex_enabled;
	this->client_max_body_size = src.client_max_body_size;
	this->error_pages          = src.error_pages;
	this->indices              = src.indices;
	this->root_path            = src.root_path;
}

void Configurable::applyDirectives(const std::vector<Directive>& directives, const Logger& l) {
	std::vector<Directive>::const_iterator start = directives.begin();
	std::vector<Directive>::const_iterator end   = directives.end();
	std::vector<Directive>::const_iterator it    = start;

	l.log("Applying configurable options");

	it = std::find(start, end, "autoindex");
	if (it != end) {
		this->applyAutoindexDirective(*it);
	}

	it = std::find(start, end, "client_max_body_size");
	if (it != end) {
		this->applyClientMaxBodySizeDirective(*it);
	}

	it = std::find(start, end, "index");
	if (it != end) {
		this->applyIndexDirective(*it);
	}

	it = std::find(start, end, "root");
	if (it != end) {
		this->applyRootPathDirective(*it);
	}

	it = start;
	while (it != end) {
		if (*it == "error_page")
			this->applyErrorPageDirective(*it);
		++it;
	}
};

bool Configurable::getAutoindexEnabled( void ) const {
	return this->autoindex_enabled;
}

size_t Configurable::getClientMaxBodySize( void ) const {
	return this->client_max_body_size;
}

const std::unordered_map<int, std::string>& Configurable::getErrorPages( void ) const {
	return this->error_pages;
}

const std::vector<std::string>& Configurable::getIndices( void ) const {
	return this->indices;
}

const std::string& Configurable::getRootPath( void ) const {
	return this->root_path;
}

const std::string& Configurable::getErrorPageForCode(int code) const {
	std::unordered_map<int, std::string>::const_iterator it = this->error_pages.find(code);
	if (it == this->error_pages.end()) {
		throw std::invalid_argument("Error code " + std::to_string(code) + " was requested but is not present in map");
	}
	return it->second;
}

void Configurable::applyAutoindexDirective(const Directive& d) {
	this->autoindex_enabled = (d.getArguments()[0] == "on");
}

void Configurable::applyClientMaxBodySizeDirective(const Directive& d) {
	this->client_max_body_size = size_to_int(d.getArguments()[0]);
}

void Configurable::applyErrorPageDirective(const Directive& d) {
	std::vector<std::string>::const_iterator it  = d.getArgumentsIterator();
	std::vector<std::string>::const_iterator end = (d.getArgumentsEnd())--;

	while (it != end)
	{
		this->error_pages[std::stoi(*it)] = *end;
		++it;
	}
}

void Configurable::applyIndexDirective(const Directive& d) {
	this->indices = d.getArguments();
}

void Configurable::applyRootPathDirective(const Directive& d) {
	this->root_path = d.getArguments()[0];
}
