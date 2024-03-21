#pragma once

#include <string>

#include "Directive.hpp"

class ConfigReturn {
	public:
		virtual ~ConfigReturn() { };

	protected:
		ConfigReturn( void ) { };

		void applyReturnDirective(const Directive& directive);

		bool        return_active      = false;
		int         return_status_code = 0;
		std::string return_body;
};
