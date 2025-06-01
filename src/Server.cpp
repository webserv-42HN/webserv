#include "../includes/Server.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#include "../includes/utils.hpp"
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <set>

#define BUF_SIZE 8194

volatile sig_atomic_t gSignal = 1;

bool Server::running = true;

Server::Server(std::vector<ServerConfig> config) : config(config) {
	setupPorts();
}

void Server::run() {
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	mainLoop();
	cleanup();
}

void Server::signalHandler(int signum) {
	(void)signum;
	running = false;
}

void stopLoop(int) {
	gSignal = 0;
}

void Server::mainLoop() {
	while (running) {
		int poll_count = poll(poll_fds.data(), poll_fds.size(), 1000);
		if (poll_count < 0) {
			if (errno == EINTR)
				continue;
			perror("poll");
			break;
		}

		for (size_t i = 0; i < poll_fds.size(); i++) {
			if (poll_fds[i].revents & POLLIN) {
				if (std::find(ss_Fds.begin(), ss_Fds.end(), poll_fds[i].fd) != ss_Fds.end()) {
					handleNewConnection(poll_fds[i].fd);
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

void Server::handleNewConnection(int listen_id) {
	int client_fd = accept(listen_id, nullptr, nullptr);
		if (client_fd < 0) {
			perror("accept");
			return ;
		}
	struct pollfd pfd = {client_fd, POLLIN, 0};
	poll_fds.push_back(pfd);
	clientConfigs[client_fd] = serverSockets[listen_id];
}

void Server::handleClientData(int client_fd) {
	Response res(config);

	std::cout << "DEBUG: handleClientData" << std::endl;
	char buf[BUF_SIZE];
	ssize_t nread = recv(client_fd, buf, BUF_SIZE - 1, 0);
	if (nread <= 0) {
		closeClient(client_fd);
		return;
	}

	ClientSession& session = client_sessions[client_fd];
	session.buffer.append(buf, nread);

	size_t header_end = session.buffer.find("\r\n\r\n");
	if (!session.headers_received && header_end != std::string::npos) {
		session.headers_received = true;
		header_end += 4;
		std::string headers = session.buffer.substr(0, header_end);
		session.content_length = res.getContentLength(headers);
	}

	if (session.headers_received && session.buffer.size() >= header_end + session.content_length) {
		std::string full_request = session.buffer;

		std::string response;
		if (res.isMalformedRequest(full_request)) {
			response = res.getErrorResponse(400);
		} else {
			res.parseRequest(full_request);
			response = res.routing(res.getRequestLine().method, res.getRequestLine().url);
		}
		responses[client_fd] = response;
		client_sessions.erase(client_fd);

		for (auto& pfd : poll_fds) {
			if (pfd.fd == client_fd) {
				pfd.events = POLLOUT;
				pfd.revents = 0;
				break;
			}
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

	responses.erase(client_fd);

	for (auto& pfd : poll_fds) {
		if(pfd.fd == client_fd) {
			pfd.events = POLLIN;
			break;
		}
	}
}

void Server::closeClient(int client_fd){
	close (client_fd);

	for (std::vector<struct pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end(); ++it) {
		if (it->fd == client_fd) {
			poll_fds.erase(it);
			break;
		}
	}
	clientConfigs.erase(client_fd);
}

void Server::cleanup () {
	for (size_t i = 0; i < poll_fds.size(); i++) {
		close(poll_fds[i].fd);
	}
	poll_fds.clear();
}

void Server::setupPorts() {
	std::set<int> seenPorts;
	for (std::vector<ServerConfig>::iterator it = config.begin(); it != config.end(); ++it) {
		if (!seenPorts.insert(it->port).second) {
			std::cerr << "Error: Port " << it->port << " is declared more than once in the config.\n";
			exit(1);
		}
	}

	std::vector<ServerConfig>::iterator it = config.begin();
	std::vector<ServerConfig>::iterator it_end = config.end();

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
				perror("fcntl");
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
			std::cout << "Middle Serv running on the port " << it->port << std::endl;
			struct pollfd pollfd = {it->sock_fd, POLLIN, 0};
			poll_fds.push_back(pollfd);
			ss_Fds.push_back(it->sock_fd);
			uniqPorts.push_back(it->port);
			serverSockets[it->sock_fd] = *it;
			//std::cout << "we pushed: "  << "sock_fd: " << it->sock_fd << ", port: " << it->port << std::endl;
		}
		++it;
	}
}

