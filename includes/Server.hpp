#pragma once
#include <string>
#include <vector>
#include <poll.h>

class Server {
	public:
		Server(int port, const std::string& root);
		void run();
		static void signalHandler(int signum);

	private:
		void setupSocket();
		void mainLoop();
		void cleanup();

		void handleNewConnection();
		void handleClientData(int client_fd);
		void closeClient(int client_fd);

		int port;
		std::string root;
		int server_fd;
		std::vector<struct pollfd> poll_fds;
		static bool running;
};