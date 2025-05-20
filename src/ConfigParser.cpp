#include "ConfigParser.hpp"
#include <fstream>

std::map<std::string, std::string> ConfigParser::parse(const std::string& path) {
	std::ifstream file(path.c_str());
	std::map<std::string, std::string> config;
	std::string key, value;

	while(file >> key >> value) config[key] = value;
	return config;
}