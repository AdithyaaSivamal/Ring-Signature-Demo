#include "util.h"
#include <fstream>
#include <sstream>

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Could not open " + filename);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

std::vector<std::string> readLines(const std::string& filename, int numLines) {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Could not open " + filename);
    std::vector<std::string> lines;
    std::string line;
    for (int i = 0; i < numLines && std::getline(file, line); ++i) {
        lines.push_back(line);
    }
    file.close();
    return lines;
}

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Could not write to " + filename);
    file << content;
    file.close();
}
