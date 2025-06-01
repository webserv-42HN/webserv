#include "../includes/Config_Manager.hpp"
#include "../includes/Server.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
    return (0);
  }
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