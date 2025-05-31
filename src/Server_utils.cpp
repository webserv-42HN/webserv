#include "../includes/Server.hpp"

std::string Server::getHostFromHeaders(const std::string& headers) {
    size_t pos = headers.find("Host:");
    if (pos == std::string::npos) {
        return "";
    }
    size_t start = headers.find_first_not_of(" \t", pos + 5);
    size_t end = headers.find_first_of("\r\n", start);
    std::string host = headers.substr(start, end - start);

    // Remove port if present (e.g., "localhost:8081" -> "localhost")
    size_t colon = host.find(':');
    if (colon != std::string::npos) {
        host = host.substr(0, colon);
    }
    return host;
}

const ServerConfig* Server::getServerConfigByHost(const std::vector<ServerConfig>& configs,
                                            const std::string& host, int port) {
    for (size_t i = 0; i < configs.size(); ++i) {
        if (configs[i].port == port) {
            for (size_t j = 0; j < configs[i].server_names.size(); ++j) {
                if (configs[i].server_names[j] == host)
                    return &configs[i];
            }
        }
    }
    // Fallback: return first config for this port if no host matches
    for (size_t i = 0; i < configs.size(); ++i) {
        if (configs[i].port == port)
            return &configs[i];
    }
    return nullptr;
}

int Server::getPortFromHeaders(const std::string& headers) {
    size_t pos = headers.find("Host:");
    if (pos == std::string::npos) {
        return -1; // No Host header found
    }
    size_t start = headers.find_first_not_of(" \t", pos + 5);
    size_t end = headers.find_first_of("\r\n", start);
    std::string host_port = headers.substr(start, end - start);

    // Extract port if present
    size_t colon = host_port.find(':');
    if (colon != std::string::npos) {
        return std::stoi(host_port.substr(colon + 1));
    }
    return 80; // Default HTTP port
}