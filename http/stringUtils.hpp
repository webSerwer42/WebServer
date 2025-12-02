#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

std::string normalizePolishChars(const std::string& text);
void getWords(const std::string& inputToSplit, std::vector<std::string>& results, std::string words);
std::string wordToLower(const std::string& str);

// Additional utility functions for HTTP parsing
std::string trim(const std::string& str);
std::vector<std::string> split(const std::string& str, char delimiter);
std::string toLower(const std::string& str);
size_t stringToSize(const std::string& str);

//