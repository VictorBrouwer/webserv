#pragma once

#include<string>
#include<iostream>

#include "Logger.hpp"

namespace Color {
    const std::string Reset = "\033[0m";
    const std::string Black = "\033[30m";
    const std::string Red = "\033[31m";
    const std::string Green = "\033[32m";
    const std::string Yellow = "\033[33m";
    const std::string Blue = "\033[34m";
    const std::string Magenta = "\033[35m";
    const std::string Cyan = "\033[36m";
    const std::string White = "\033[37m";
} // namespace Color

// Function declarations
void log(const std::string& message, const std::string& color) noexcept;
void log(const std::string &message, const LogLevel level = L_Debug) noexcept;
void exitWithError(const std::string &errorMessage);

std::string vector_to_string(const std::vector<std::string>& vector);
size_t      size_to_int(const std::string& str);
std::string strip(const std::string &input, std::string strip_chars);
std::vector<std::string> split(const std::string &input,
								  const std::string &delimiter,
								  std::size_t max_count = -1);
std::string 		joinPath(std::vector<std::string> paths, std::string delimeter);