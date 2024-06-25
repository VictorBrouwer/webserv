#pragma once

#include<string>

#define CRLF (std::string("\r\n"))
#define CRLFCRLF (std::string("\r\n\r\n"))
#define VERSION (std::string("HTTP/1.1"))
#define WHITESPACE (std::string("\r\n\t "))
#define HTML_FOOTER \
"		</pre>\n" \
"		<hr>\n" \
"	</body>\n" \
"</html>\n"


#define E_UNKNOWN_DIRECTIVE   (std::string("Unknown directive"))
#define E_DUPLICATE_DIRECTIVE (std::string("Duplicate directive"))
#define E_WRONG_CONTEXT       (std::string("Directive is not allowed in this context"))
#define E_UNKNOWN_ARGUMENT    (std::string("Unknown argument for directive"))
#define E_BLOCK_NOT_CLOSED    (std::string("Unclosed block for directive"))
#define E_INCORRECT_FORMAT    (std::string("Incorrectly formatted line"))
#define E_MISSING_DIRECTIVE   (std::string("Expected directive is missing"))

const static std::string DEFAULT_ERR_PAGE = R"(
<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Error</title>
	<style>
		body {
			font-family: Arial, sans-serif;
			background-color: #f4f4f4;
			color: #333;
			margin: 0;
			padding: 0;
		}
		.container {
			max-width: 600px;
			margin: 50px auto;
			padding: 20px;
			background-color: #fff;
			border-radius: 5px;
			box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
		}
		h1 {
			color: #e002fd;
		}
		p {
			margin-bottom: 20px;
		}
		a {
			color: #007bff;
			text-decoration: none;
		}
		a:hover {
			text-decoration: underline;
		}
	</style>
</head>
<body>
	<div class="container">
		<h1>Oops!</h1>
		<p>There was an error processing your request.</p>
	</div>
</body>
</html>
)";
