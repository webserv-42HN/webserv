#include "config_manager.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  // Default configuration file
  std::string configFile = "webserv.conf";
  
  // Allow custom config file via command line
  if (argc > 1) {
      configFile = argv[1];
  }
  
  // Create config manager and load configuration
  ConfigManager configManager;
  if (!configManager.loadFromFile(configFile)) {
      std::cerr << "Configuration error: " << configManager.getErrorMessage() << std::endl;
      return 1;
  }
  
  // Print loaded configuration (optional)
  configManager.printConfigs();
  
  // Now you can pass configManager.getServerConfigs() to other modules
  
  return 0;
}
