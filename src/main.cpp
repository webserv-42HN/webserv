#include "../includes/config_manager.hpp"
#include "../includes/Server.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2)
    return (0);
  std::string configFile = argv[1];
  ConfigManager configManager;
  if (!configManager.loadFromFile(configFile)) {
    std::cerr << "Configuration error: " << configManager.getErrorMessage() << std::endl;
    return 1;
  }
  // configManager.printConfigs();
	Server myServer(configManager.getServerConfigs());

	myServer.run();

	return(0);
}