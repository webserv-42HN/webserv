#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "utils.hpp"
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>

Server::Server(int p, const std::string& r) : port(p), root(r), server_fd(-1) {}

bool Server::running = true;

void Server::mainLoop() {
	while (running) {
		int poll_count = poll(poll_fds.data(), poll_fds.size(), 1000);
		if (poll_count < 0) {
			perror("poll");
			break;
		}

		for (size_t i = 0; i < poll_fds.size(); i++) {
			if (poll_fds[i].revents & POLLIN) {
				if (poll_fds[i].fd == server_fd) {
					handleNewConnection();
				} else {
					handleClientData(poll_fds[i].fd);
				}
			}
		}
	}
}

void Server::handleNewConnection() {
	int client_fd = accept(server_fd, nullptr, nullptr);
		if (client_fd < 0) {
			perror("accept");
			return ;
		}
	struct pollfd pfd = {client_fd, POLLIN, 0};
	poll_fds.push_back(pfd);
}

void Server::handleClientData(int client_fd) {
	char buffer[1024] = {0};
	ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_read <= 0) {
		closeClient(client_fd);
		return ;
	}
	std::cout << "Received request from client :\n" << buffer << std::endl;

	Request req = Request::parse(buffer);
	std::string path = root + (req.path == "/" ? "/index.html" : req.path);
	std::string body = read_file(path);
	std::string response;

	if (req.method != "GET") {
		response = Response::build("<h1>405 Method Not Allowed</h1>", "text/html", 405, "Method Not Allowed");
	} else if (body.empty()) {
		body = read_file(root + "/404.html");
		response = Response::build(body, "text/html", 404, "Not Found");
	} else {
		response = Response::build(body);
	}

	send(client_fd, response.c_str(), response.length(), 0);

	closeClient(client_fd);
}

void Server::closeClient(int client_fd){
	close (client_fd);

	for (std::vector<struct pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end(); ++it) {
		if (it->fd == client_fd) {
			poll_fds.erase(it);
			break;
		}
	}
}

void Server::cleanup () {
	for (size_t i = 0; i < poll_fds.size(); i++) {
		close(poll_fds[i].fd);
	}
	poll_fds.clear();

	if (server_fd != -1) {
		close(server_fd);
		server_fd = -1;
	}
}

void Server::setupSocket() {
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == 0) {
		std::cerr << "Failed to create socket\n";
		exit(1);
	}

	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
		perror("setsockopt");
		exit(1);
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		std::cerr << "Bind failed\n";
		return ;
	}

	if (listen(server_fd, 10) < 0) {
		std::cerr << "Listen failed\n";
		return ;
	}
	std::cout << "Mini Serv running on the port " << port << std::endl;

	struct pollfd pfd = {server_fd, POLLIN, 0};
	poll_fds.push_back(pfd);
}

void Server::run() {
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	setupSocket();
	mainLoop();
	cleanup();
}

void Server::signalHandler(int signum) {
	std::cout << "Shuttimg down the server..." << std::endl;
	running = false;
}