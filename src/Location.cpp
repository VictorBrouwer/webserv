#include "Location.hpp"

Location::Location(const Directive& directive, ConfigShared* shared, const Logger& logger) : ConfigShared(shared), l(logger) {
	l.log("Setting up location for " + directive.getArguments()[0]);

	this->uri = directive.getArguments()[0];

	// Apply shared directives
	this->applySharedDirectives(directive.getSubdirectives(), l);

	auto start = directive.getSubdirectivesIterator();
	auto end   = directive.getSubdirectivesEnd();
	auto it    = start;

	// Apply return directive
	it = std::find(start, end, "return");
	if (it != end) {
		this->applyReturnDirective(*it);
	}

	// Apply location-specific directives (limit_except, nested locations)

	it = std::find(start, end, "limit_except");
	if (it != end) {
		this->allowed_methods = it->getArguments();
	}

	it = start;
	while (it != end) {
		if (*it == "location")
			this->locations.push_back(Location(*it, (ConfigShared*) this, l));
		++it;
	}
}

std::string& Location::getUri()
{
	return (this->uri);
}

bool Location::operator==(const std::string& uri) const {
	return (this->uri == uri);
}

bool Location::checkMethod(HTTPMethod method)
{
	std::string s_method;
	switch (method)
	{
	case HTTPMethod::GET:
		s_method = "GET";
		break;
	case HTTPMethod::POST:
		s_method = "POST";
		break;
	case HTTPMethod::DELETE:
		s_method = "DELETE";
		break;
	default:
		s_method = "UNDEFINED";
		break;
	}
	for (const auto &met : allowed_methods)
	{
		if (met == s_method)
			return true;
	}
	return false;
}

bool Location::getReturnActive()
{
	return this->return_active;
}

int Location::getReturnStatusCode()
{
	return this->return_status_code;
}

std::string& Location::getReturnBody()
{
	return this->return_body;
}

Location::~Location() { }
