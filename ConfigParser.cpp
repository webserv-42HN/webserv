#include "ConfigParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

bool ConfigParser::parse(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << path << std::endl;
        return false;
    }

    std::string line;
    ServerConfigs currentConfig = {};
    bool hasPort = false, hasRoot = false;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (!(iss >> key >> value)) {
            continue; // skip malformed lines
        }

        if (key == "port") {
            currentConfig.port = std::stoi(value);
            hasPort = true;
        } else if (key == "root") {
            currentConfig.root = value;
            hasRoot = true;
        }

        if (hasPort && hasRoot) {
            configs.push_back(currentConfig);
            currentConfig = {}; // reset for next block
            hasPort = hasRoot = false;
        }
    }

    return true;
}

const std::vector<ServerConfigs>& ConfigParser::getConfigs() const {
    return configs;
}
