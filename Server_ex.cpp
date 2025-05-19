#include "Server.hpp"
#include "HttpRequest.hpp"
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <poll.h>
#include <cstring>
#include <ctime>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <map>
#include <algorithm>

volatile sig_atomic_t gServerSignal = 1;

Server::Server(std::vector<ServerParser> config) : servers(config) {
    createSockets();
    startPolling();
}

Server::~Server() {
    for (auto& pfd : pollFds) {
        std::cout << "Closing fd: " << pfd.fd << std::endl;
        close(pfd.fd);
    }
}

void stopServer(int) {
    gServerSignal = 0;
    std::cout << "Received SIGINT, stopping server." << std::endl;
}

void Server::createSockets() {
    std::vector<int> usedPorts;
    for (auto& srv : servers) {
        srv.sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (srv.sockfd < 0)
            throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));

        int opt = 1;
        setsockopt(srv.sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        fcntl(srv.sockfd, F_SETFL, O_NONBLOCK);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(srv.listen);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (std::find(usedPorts.begin(), usedPorts.end(), srv.listen) == usedPorts.end()) {
            if (bind(srv.sockfd, (sockaddr*)&addr, sizeof(addr)) < 0)
                throw std::runtime_error("Bind failed: " + std::string(strerror(errno)));
            if (listen(srv.sockfd, SOMAXCONN) < 0)
                throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));

            pollfd pfd = {srv.sockfd, POLLIN, 0};
            pollFds.push_back(pfd);
            serverFds.push_back(srv.sockfd);
            usedPorts.push_back(srv.listen);
        }
    }
}

void Server::startPolling() {
    signal(SIGINT, stopServer);
    signal(SIGPIPE, SIG_IGN);

    while (gServerSignal) {
        if (poll(pollFds.data(), pollFds.size(), 0) < 0 && gServerSignal)
            throw std::runtime_error("Poll failed");

        for (size_t i = 0; i < pollFds.size(); ++i) {
            if (pollFds[i].revents & POLLIN) handlePollin(pollFds[i]);
            if (pollFds[i].revents & POLLOUT) handlePollout(pollFds[i]);
            if (pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
                clients[pollFds[i].fd].closeConnection = true;

            if (isClient(pollFds[i].fd) && checkStale(pollFds[i])) i--;
        }
    }
}

void Server::acceptNewConnection(int serverFd) {
    sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    int clientFd = accept(serverFd, (sockaddr*)&clientAddr, &addrLen);
    if (clientFd < 0) return;
    fcntl(clientFd, F_SETFL, O_NONBLOCK);

    pollfd pfd = {clientFd, POLLIN, 0};
    pollFds.push_back(pfd);
    clientFds.push_back(clientFd);
    clients[clientFd] = clientState{clientFd};
    time(&clients[clientFd].lastEventTime);

    std::cout << "Accepted connection: " << clientFd << std::endl;
}

bool Server::isServer(int fd) {
    return std::find(serverFds.begin(), serverFds.end(), fd) != serverFds.end();
}

bool Server::isClient(int fd) {
    return std::find(clientFds.begin(), clientFds.end(), fd) != clientFds.end();
}

void Server::handlePollin(pollfd& pfd) {
    if (isServer(pfd.fd)) {
        acceptNewConnection(pfd.fd);
        return;
    }

    char buf[4096 * 4] = {};
    ssize_t n = recv(pfd.fd, buf, sizeof(buf), 0);
    if (n <= 0) {
        clients[pfd.fd].closeConnection = true;
        return;
    }

    clients[pfd.fd].bytesRead = n;
    clients[pfd.fd].readString.assign(buf, n);
    HttpRequest::requestBlock(clients[pfd.fd], servers);
    time(&clients[pfd.fd].lastEventTime);

    HttpResponse response;
    if ((clients[pfd.fd].method != DEFAULT) && clients[pfd.fd].flagHeaderRead) {
        if (clients[pfd.fd].method == POST && !clients[pfd.fd].flagBodyRead) return;
        clients[pfd.fd].writeString = response.respond(clients[pfd.fd]);
        pfd.events = POLLOUT;
    }
}

void Server::handlePollout(pollfd& pfd) {
    if (clients[pfd.fd].writeString.empty()) {
        pfd.events = POLLIN;
        return;
    }

    ssize_t sent = send(pfd.fd, clients[pfd.fd].writeString.c_str(), clients[pfd.fd].writeString.size(), 0);
    if (sent <= 0) {
        clients[pfd.fd].closeConnection = true;
        return;
    }

    clients[pfd.fd].writeString.erase(0, sent);
    if (clients[pfd.fd].writeString.empty()) {
        if (!clients[pfd.fd].isKeepAlive)
            clients[pfd.fd].closeConnection = true;
        pfd.events = POLLIN;
        clients[pfd.fd].clear();
    }
}

bool Server::checkStale(pollfd& pfd) {
    time_t now;
    time(&now);
    auto& client = clients[pfd.fd];
    if (client.closeConnection || difftime(now, client.lastEventTime) > client.serverData.keepalive_timeout) {
        close(pfd.fd);
        clientFds.erase(std::remove(clientFds.begin(), clientFds.end(), pfd.fd), clientFds.end());
        clients.erase(pfd.fd);
        pollFds.erase(std::remove_if(pollFds.begin(), pollFds.end(), [&](pollfd const& e) { return e.fd == pfd.fd; }), pollFds.end());
        std::cout << "Closed client: " << pfd.fd << std::endl;
        return true;
    }
    return false;
}
