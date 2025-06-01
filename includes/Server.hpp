#pragma once
#include <string>
#include <vector>
#include <poll.h>
#include <unordered_map>
#include <map>
#include "config_manager.hpp"
#include <fcntl.h>

struct ClientSession {
	std::string buffer;
	bool headers_received = false;
	int content_length = 0;
};

class Server {
	private:
		std::vector<ServerConfig> config;
		std::vector<int> ss_Fds;
		std::map<int, ServerConfig> serverSockets;
		std::map<int, ServerConfig> clientConfigs;
		std::vector<struct pollfd> poll_fds;
		std::vector<int> uniqPorts;
		std::map<int, ClientSession> client_sessions;

	public:
		Server(std::vector<ServerConfig> config);

		void setupPorts();

		void run();
		static void signalHandler(int signum);

		void mainLoop();
		void cleanup();

		void handleNewConnection(int listen_id);
		void handleClientData(int client_fd);
		void handleClientWrite(int client_fd);
		void closeClient(int client_fd);

		static bool running;
		std::unordered_map<int, std::string> responses;
};

