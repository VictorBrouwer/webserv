#pragma once

#include<string>
#define CRLF (std::string("\r\n"))
#define CRLFCRLF (std::string("\r\n\r\n"))
#define VERSION (std::string("HTTP/1.1"))
#define WHITESPACE (std::string("\r\n\t "))

#define E_UNKNOWN_DIRECTIVE   (std::string("Unknown directive"))
#define E_DUPLICATE_DIRECTIVE (std::string("Duplicate directive"))
#define E_WRONG_CONTEXT       (std::string("Directive is not allowed in this context"))
#define E_UNKNOWN_ARGUMENT    (std::string("Unknown argument for directive"))
#define E_BLOCK_NOT_CLOSED    (std::string("Unclosed block for directive"))
#define E_INCORRECT_FORMAT    (std::string("Incorrectly formatted line"))
#define E_MISSING_DIRECTIVE   (std::string("Expected directive is missing"))
