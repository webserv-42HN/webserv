#pragma once
#include <string>
#include <vector>
#include <poll.h>
#include <unordered_map>
// #include "ConfigParser.hpp" //to remove
#include "config_manager.hpp"
#include <fcntl.h>

class Server {
	private:
		std::vector<ServerConfig> config;
		std::vector<int> ss_Fds;
		std::vector<struct pollfd> poll_fds;
		std::vector<int> uniqPorts;

	public:
		Server(std::vector<ServerConfig> config);
		// ~Server();

		void setupPorts(); //initial step when we go through the results of the parser,
		//look for the unique ports and create socket for each port we gonna listen on


		void run();
		static void signalHandler(int signum);

		void mainLoop();
		void cleanup();

		void handleNewConnection();
		void handleClientData(int client_fd);
		void handleClientWrite(int client_fd);
		void closeClient(int client_fd);

		int port;
		std::string root;
		int server_fd;

		static bool running;
		std::unordered_map<int, std::string> responses;
};