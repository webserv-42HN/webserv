#pragma once
#include <string>
#include <vector>
#include <poll.h>
#include <unordered_map>
#include <map>
#include "ConfigParser.hpp"
#include <fcntl.h>

class Server {
	private:
		std::vector<ServerConfigs> config;
		std::vector<int> ss_Fds; //delete later, it's gonna be a map
		std::map<int, ServerConfigs> serverSockets;
		std::map<int, ServerConfigs> clientConfigs;
		std::vector<struct pollfd> poll_fds;
		std::vector<int> uniqPorts;

	public:
		Server(std::vector<ServerConfigs> config);
		// ~Server();

		void setupPorts(); //initial step when we go through the results of the parser,
		//look for the unique ports and create socket for each port we gonna listen on


		void run();
		static void signalHandler(int signum);

		void mainLoop();
		void incoming(int fd);
		void cleanup();

		void handleNewConnection(int listen_id);
		void handleClientData(int client_fd);
		void handleClientWrite(int client_fd);
		void closeClient(int client_fd);


		static bool running;
		std::unordered_map<int, std::string> responses;
};