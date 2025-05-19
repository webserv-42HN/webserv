#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "utils.hpp"
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>

Server::Server(std::vector<ServerConfigs> config) : config(config) {
	Server::setupPorts();
}

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

			if (poll_fds[i].revents & POLLOUT) {
				handleClientWrite(poll_fds[i].fd);
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

	responses[client_fd] = response;

	for (auto& pfd : poll_fds) {
		if (pfd.fd == client_fd) {
			pfd.events = POLLOUT;
			pfd.revents = 0;

			break;
		}
	}
}

void Server::handleClientWrite(int client_fd) {
	auto it = responses.find(client_fd);
	if (it == responses.end()) {
		std::cerr << "No response found for client " << client_fd << std::endl;
		closeClient(client_fd);
		return ;
	}
	const std::string& response = it->second;
	ssize_t bytes_sent = send(client_fd, response.c_str(), response.length(), 0);
	if (bytes_sent <= 0) {
		perror("send");
		closeClient(client_fd);
		responses.erase(client_fd);
		return ;
	}
	std::cout << "Sent response to client :\n" << response << std::endl;

	responses.erase(client_fd);
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

void Server::setupPorts() {
	std::vector<ServerConfigs>::iterator it = config.begin();
	std::vector<ServerConfigs>::iterator it_end = config.end();

	while (it != it_end) {
		if(std::find(uniqPorts.begin(), uniqPorts.end(), it->port) == uniqPorts.end()) {

			it->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
			if(it->sock_fd  < 0) {
				std::cerr << "Failed to create socket\n";
				exit(1);
			}
			int opt = 1;
			if (setsockopt(it->sock_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
				perror("setsockopt");
				exit(1);
			}

			if (fcntl(it->sock_fd, F_SETFL, O_NONBLOCK) < 0) {
				perror("fctnl");
				exit(1);
			}

			sockaddr_in addr;
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = INADDR_ANY;
			addr.sin_port = htons(it->port);

			if (bind(it->sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
				std::cerr << "Bind failed\n";
				return ;
			}

			if (listen(it->sock_fd, SOMAXCONN) < 0) {
				std::cerr << "Listen failed\n";
				return ;
			}
			std::cout << "Middle Serv running on the port " << port << std::endl;
			struct pollfd pollfd = {it->sock_fd, POLLIN, 0};
			poll_fds.push_back(pollfd);
			ss_Fds.push_back(it->sock_fd);
			uniqPorts.push_back(it->port);
			std::cout << "we pushed: "  << "sock_fd: " << it->sock_fd << ", port: " << it->port << std::endl;
		}
		++it;
	}

	std::cout << "=== Results of setupPorts() ===" << std::endl;

// Print unique ports
std::cout << "[Unique Ports]" << std::endl;
for (std::vector<int>::iterator it = uniqPorts.begin(); it != uniqPorts.end(); ++it) {
	std::cout << "Port: " << *it << std::endl;
}

// Print server socket file descriptors
std::cout << "\n[Server Socket FDs]" << std::endl;
for (std::vector<int>::iterator it = ss_Fds.begin(); it != ss_Fds.end(); ++it) {
	std::cout << "Socket FD: " << *it << std::endl;
}

// Print poll file descriptors
std::cout << "\n[Poll FDs]" << std::endl;
for (std::vector<struct pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end(); ++it) {
	std::cout << "pollfd { fd: " << it->fd
	          << ", events: " << it->events
	          << ", revents: " << it->revents << " }" << std::endl;
}

}

void Server::run() {
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	setupPorts();
	mainLoop();
	cleanup();
}

void Server::signalHandler(int signum) {
	(void)signum;
	std::cout << "Shuttimg down the server..." << std::endl;
	running = false;
}