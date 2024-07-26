#include "ConfigShared.hpp"
#include "HelperFuncs.hpp"

ConfigShared::ConfigShared(ConfigShared* src) {
	this->indices              = src->getIndices();
	this->root_path            = src->getRootPath();
	this->upload_dir		   = src->getUploadDir();
	this->error_pages          = src->getErrorPages();
	this->autoindex_enabled    = src->getAutoindexEnabled();
	this->client_max_body_size = src->getClientMaxBodySize();
}

void ConfigShared::applySharedDirectives(const std::vector<Directive>& directives, const Logger& l) {
	std::vector<Directive>::const_iterator start = directives.begin();
	std::vector<Directive>::const_iterator end   = directives.end();
	std::vector<Directive>::const_iterator it    = start;

	l.log("Applying shared options");

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
	it = std::find(start, end, "upload_dir");
	if (it != end) {
		this->applyUploadDirDirective(*it);
	}

	it = start;
	while (it != end) {
		if (*it == "error_page")
			this->applyErrorPageDirective(*it);
		++it;
	}
};

bool ConfigShared::getAutoindexEnabled( void ) const {
	return this->autoindex_enabled;
}

size_t ConfigShared::getClientMaxBodySize( void ) const {
	return this->client_max_body_size;
}

const std::unordered_map<int, std::string>& ConfigShared::getErrorPages( void ) const {
	return this->error_pages;
}

const std::vector<std::string>& ConfigShared::getIndices( void ) const {
	return this->indices;
}

const std::string& ConfigShared::getRootPath( void ) const {
	return this->root_path;
}

const std::string& ConfigShared::getUploadDir( void ) const {
	return this->upload_dir;
}

const std::string& ConfigShared::getErrorPageForCode(int code) const {
	std::unordered_map<int, std::string>::const_iterator it = this->error_pages.find(code);
	if (it == this->error_pages.end()) {
		throw code;
	}
	return it->second;
}

void ConfigShared::applyAutoindexDirective(const Directive& d) {
	this->autoindex_enabled = (d.getArguments()[0] == "on");
}

void ConfigShared::applyClientMaxBodySizeDirective(const Directive& d) {
	this->client_max_body_size = size_to_int(d.getArguments()[0]);
}

void ConfigShared::applyErrorPageDirective(const Directive& d) {
	std::vector<std::string>::const_iterator it  = d.getArgumentsIterator();
	// std::vector<std::string>::const_iterator end = (d.getArgumentsEnd())--;
	std::string status_code = *it;
	it++;
	std::string path = *it;

	this->error_pages[std::stoi(status_code)] = path;
}

void ConfigShared::applyIndexDirective(const Directive& d) {
	this->indices = d.getArguments();
}

void ConfigShared::applyRootPathDirective(const Directive& d) {
	this->root_path = d.getArguments()[0];
}

void ConfigShared::applyUploadDirDirective(const Directive& d) {
	this->upload_dir = d.getArguments()[0];
}
