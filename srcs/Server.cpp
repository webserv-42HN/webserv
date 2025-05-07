#include "../includes/Server.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"

Server::Server(const std::string& port) : port(port) {
    sfd = -1;
}

Server::~Server() {
    if (sfd != -1) {
        close(sfd);
    }
}

void Server::setup_listening_socket() {
    struct addrinfo hints, *result, *rp;
    int s;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;        // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // Provides sequenced, reliable, two-way connection-based byte streams.
    hints.ai_flags = AI_PASSIVE;        // For wildcard IP address
    hints.ai_protocol = 0;              // Any protocol

    s = getaddrinfo(NULL, port.c_str(), &hints, &result);
    if (s != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(s) << std::endl;
        exit(EXIT_FAILURE);
    }
    // Try each address until we successfully bind
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1) {
            continue;
        }
        // Add socket options in your server code to allow reuse of the address
        // It allows you to restart the server without waiting
        int opt = 1;
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        // Set the socket to non-blocking mode
        fcntl(sfd, F_SETFL, O_NONBLOCK);
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;  // Success
        close(sfd);
    }
    if (rp == NULL) {
        std::cerr << "Could not bind" << std::endl;
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);
    if (listen(sfd, SOMAXCONN) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

void Server::start() {
    setup_listening_socket();

    // Pre-size the vector to hold MAX_CLIENTS + 1 (including listening socket)
    fds.resize(MAX_CLIENTS + 1);
    // Initialize the listening socket
    fds[0].fd = sfd;
    fds[0].events = POLLIN;
    // Initialize client slots
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }
    std::cout << "Server is listening on port " << port << "...\n";
    while (true) {
        // Use fds.size() instead of nfds
        int poll_count = poll(fds.data(), fds.size(), -1);
        if (poll_count == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }
        // Check for new connections
        if (fds[0].revents & POLLIN) {
            struct sockaddr_storage peer_addr;
            socklen_t peer_addr_len = sizeof(peer_addr);
            int cfd = accept(sfd, (struct sockaddr *)&peer_addr, &peer_addr_len);
            if (cfd != -1) {
                bool added = false;
                for (size_t i = 1; i < fds.size(); i++) {
                    if (fds[i].fd == -1) {
                        fds[i].fd = cfd;
                        fds[i].events = POLLIN;
                        added = true;
                        break;
                    }
                }
                if (!added) {
                    std::cout << "Too many clients\n";
                    close(cfd);
                }
            }
        }
        // Check client sockets
        for (size_t i = 1; i < fds.size(); i++) {
            if (fds[i].fd != -1 && fds[i].revents & POLLIN) 
                handle_client(i);
        }
    }
}

void Server::handle_client(int i) {
    char buf[BUF_SIZE];
    std::string request;
    ssize_t nread;
    std::string response;
    Request req;
    Response res;

    nread = recv(fds[i].fd, buf, BUF_SIZE - 1, 0);
    if (nread > 0) {    
        request.append(buf, nread);
        // request = "GET /\r\n\r\n";
        if (req.isMalformedRequest(request))
            response = res.getErrorResponse(400); 
        else {
            req.parseRequest(request);
            response = res.routing(req.getRequestLine().method,
                                    req.getRequestLine().url);
            // response = res.getResponse(res.getHtmlFile(200), 200);
        }
        send(fds[i].fd, response.c_str(), response.size(), 0);
        close(fds[i].fd);
        fds[i].fd = -1;
    } else if (nread == 0 || errno != EAGAIN) {
        close(fds[i].fd);
        fds[i].fd = -1;
    }
}
