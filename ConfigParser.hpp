#pragma once

#include <string>
#include <vector>

struct ServerConfigs
{
    int port;
    std::string root;
    int sock_fd;
};

// class ConfigParser {
// private:
//     std::vector<ServerConfigs> configs;

// public:
//     bool parse(const std::string& path);
//     const std::vector<ServerConfigs>& getConfigs() const;
// };

