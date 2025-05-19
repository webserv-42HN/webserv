#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>


#define BUF_SIZE 8192
#define MAX_CLIENTS 10

class Server {
    private:
        int sfd;  // Listening socket file descriptor
        std::string port;
        std::vector<pollfd> fds;
        void handle_client(int i);
        void setup_listening_socket();
    public:
        Server(const std::string& port);
        ~Server();
        void start();
};

#endif