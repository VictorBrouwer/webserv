#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "Directive.hpp"

class Configurable {
	public:
		virtual ~Configurable( void ) { };

		bool   getAutoindexEnabled( void ) const;
		size_t getClientMaxBodySize( void ) const;
		const std::unordered_map<int, std::string>& getErrorPages( void ) const;
		const std::vector<std::string>& getIndices( void ) const;
		const std::string& getRootPath( void ) const;

		// Throws std::invalid_argument if the code is not present
		const std::string& getErrorPageForCode(int code) const;

	protected:
		Configurable() { };
		Configurable(const Configurable& src);

		void applyDirectives(const std::vector<Directive>& directives, const Logger& l);

		// Configuration member variables

		bool                                 autoindex_enabled    = false;
		size_t                               client_max_body_size = 1048576; // 1m
		std::unordered_map<int, std::string> error_pages;
		std::vector<std::string>             indices = { "index.html" };
		std::string                          root_path = "/var/www";

	private:
		void applyAutoindexDirective(const Directive& d);
		void applyClientMaxBodySizeDirective(const Directive& d);
		void applyErrorPageDirective(const Directive& d);
		void applyIndexDirective(const Directive& d);
		void applyRootPathDirective(const Directive& d);
};
