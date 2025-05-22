#include "ConfigParser.hpp"
#include "Server.hpp"
#include <iostream>

int main() {
	ConfigParser parser;
	if (parser.parse("config.conf")) {
		for (const auto& config : parser.getConfigs()) {
			std::cout << "Port: " << config.port << ", Root: " << config.root << std::endl;
		}
	}

	Server myServer(parser.getConfigs());

	myServer.run();

	return(0);
}