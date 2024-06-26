#include"ResponseHelpers.hpp"

std::string createautoIndexHTML(std::string path)
{
	std::string html = 
    "<!DOCTYPE html>\n"
    "<html>\n"
    "	<meta charset=\"UTF-8\">\n"
    "	<head>\n"
    "		<title>Index of " + path + "</title>\n"
    "		<style>\n"
    "			body {\n"
    "				background-color: gray;\n"
    "				font-size: large;\n"
    "			}\n"
    "			a {\n"
    "				padding-left: 2px;\n"
    "				margin-left: 4px;\n"
    "			}\n"
    "			button {\n"
    "				background-color: gray;\n"
    "				padding-left: 3px;\n"
    "				padding-right: 3px;\n"
    "				border: 0px;\n"
    "				margin: 2px;\n"
    "				cursor: pointer;\n"
    "			}\n"
    "			hr {\n"
    "				border-color: black;\n"
    "				background-color: black;\n"
    "				color: black;\n"
    "			}\n"
    "		</style>\n"
    "	</head>\n"
    "	<body>\n"
    "		<h1>Index of " + path + "</h1>\n"
    "		<hr>\n"
    "		<pre>\n";
	return html;
}
