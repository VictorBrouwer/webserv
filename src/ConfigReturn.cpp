#include "ConfigReturn.hpp"

void ConfigReturn::applyReturnDirective(const Directive& directive) {
	this->return_active = true;

	// This should not throw an exception, since our validator should
	// already have checked for numericality of the first argument
	this->return_status_code = std::stoi(directive.getArguments()[0]);
	this->return_body        = directive.getArguments()[1];
}
