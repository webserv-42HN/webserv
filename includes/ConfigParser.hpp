#pragma once
#include <map>
#include <string>

class ConfigParser {
	public:
		static std::map<std::string, std::string> parse(const std::string& path);
};