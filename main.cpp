#include "config_manager.hpp"
#include "Server.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
	// ConfigParser parser;
	// if (parser.parse("config.conf")) {
		// for (const auto& config : parser.getConfigs()) {
			// std::cout << "Port: " << config.port << ", Root: " << config.root << std::endl;
      // }
      // }
  if (argc != 2)
    return (0);
  std::string configFile = argv[1];
  ConfigManager configManager;
  if (!configManager.loadFromFile(configFile)) {
    std::cerr << "Configuration error: " << configManager.getErrorMessage() << std::endl;
    return 1;
  }
  // Print loaded configuration (optional)
  configManager.printConfigs();

	Server myServer(configManager.getServerConfigs());

	// serv.run();

	return(0);
}
