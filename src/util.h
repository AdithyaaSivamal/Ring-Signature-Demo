#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>

std::string readFile(const std::string& filename);
std::vector<std::string> readLines(const std::string& filename, int numLines);
std::vector<std::string> split(const std::string& s, char delimiter);
void writeFile(const std::string& filename, const std::string& content);

#endif
