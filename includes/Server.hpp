#pragma once
#include <string>
#include <vector>
#include <poll.h>
#include <unordered_map>
#include <map>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>

#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#include "../includes/utils.hpp"
#include "../includes/config_manager.hpp"

#define BUF_SIZE 8194

struct ClientSession {
	std::string buffer;
	bool headers_received = false;
	int content_length = 0;
};

// Add this struct to track CGI process state
struct CGIState {
  pid_t pid;              // CGI process ID
  int stdin_fd;           // Pipe for writing to CGI stdin
  int stdout_fd;          // Pipe for reading from CGI stdout
  std::string input_buffer; // Request body to send to CGI
  std::string output_buffer; // Accumulated CGI output
  int client_fd;          // Associated client file descriptor
  bool done;              // Whether CGI has finished
};


class Server {
	private:
		std::vector<ServerConfig> config;
		std::vector<int> ss_Fds; //delete later, it's gonna be a map
		std::map<int, ServerConfig> serverSockets;
		std::map<int, ServerConfig> clientConfigs;
		// std::vector<struct pollfd> poll_fds;
		std::vector<int> uniqPorts;
		std::map<int, ClientSession> client_sessions;

	public:
    static std::vector<struct pollfd> poll_fds;
    static std::map<int, CGIState> cgi_states; // Keyed by stdout_fd
    static int current_client_fd; // Set in main loop before handling request

		Server(std::vector<ServerConfig> config);
		// ~Server();

		void setupPorts(); //initial step when we go through the results of the parser,
		//look for the unique ports and create socket for each port we gonna listen on


		void run();
		static void signalHandler(int signum);

		void mainLoop();
		void cleanup();

		void handleNewConnection(int listen_id);
		void handleClientData(int client_fd);
		void handleClientWrite(int client_fd);
		void closeClient(int client_fd);
		
		
    	std::string processCGIOutput(const std::string& output);
		
		static bool running;
		std::unordered_map<int, std::string> responses;
	
		// Handling client data
		std::string getHostFromHeaders(const std::string& headers);
		const ServerConfig* getServerConfigByHost(const std::vector<ServerConfig>& configs,
												const std::string& host,
												int port);
		int getPortFromHeaders(const std::string& headers);
		bool receiveData(int client_fd);
		bool processHeaders(ClientSession& session);
		bool isFullRequestReceived(const ClientSession& session);
		void processRequest(int client_fd, ClientSession& session);
		void enableWriteEvents(int client_fd);
};

