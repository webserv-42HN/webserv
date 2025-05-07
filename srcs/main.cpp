#include "../includes/Server.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " port\n";
        exit(EXIT_FAILURE);
    }
    Server server(argv[1]);
    server.start();
    return 0;
}
