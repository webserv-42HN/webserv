#include "ConfigParser.hpp"
#include "Server.hpp"
#include <iostream>

int main() {
	std::map<std::string, std::string> config = ConfigParser::parse("config.conf");

	int port = std::stoi(config["port"]);
	// std::cout << "port: " << port << std::endl;
	std::string root = config["root"];

	Server serv(port, root);
	serv.run();

	return(0);
}