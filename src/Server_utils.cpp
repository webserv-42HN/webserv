#include "../includes/Server.hpp"

std::string Server::getHostFromHeaders(const std::string& headers) {
    size_t pos = headers.find("Host:");
    if (pos == std::string::npos) {
        return "";
    }
    size_t start = headers.find_first_not_of(" \t", pos + 5);
    size_t end = headers.find_first_of("\r\n", start);
    std::string host = headers.substr(start, end - start);

    size_t colon = host.find(':');
    if (colon != std::string::npos) {
        host = host.substr(0, colon);
    }
    return host;
}

const ServerConfig* Server::getServerConfigByHost(const std::vector<ServerConfig>& configs,
                                                  const std::string& host, int port) {
    // Scenario 1: Host and Port match
    if (!host.empty()) {
        for (size_t i = 0; i < configs.size(); ++i) {
            if (configs[i].port == port) {
                for (size_t j = 0; j < configs[i].server_names.size(); ++j) {
                    if (configs[i].server_names[j] == host) {
                        return &configs[i]; // Exact match found
                    }
                    else
                        return nullptr;
                }
            }
        }
        return &configs[0]; // Host provided but no match found
    }
    for (size_t i = 0; i < configs.size(); ++i) {
        if (configs[i].port == port) {
            return &configs[i]; // Fallback to first matching port config
        }
    }
    return nullptr; // No config matches port
}

int Server::getListeningPortForClient(int client_fd) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    
    if (getsockname(client_fd, (struct sockaddr*)&addr, &addr_len) == -1) {
        perror("getsockname failed");
        return -1; // error indicator
    }
    // ntohs converts network byte order to host byte order
    int port = ntohs(addr.sin_port);
    return port;
}