#include "config_manager.hpp"

// ConfigManager implementation
ConfigManager::ConfigManager() : m_hasError(false) {}
ConfigManager::~ConfigManager() {}

bool ConfigManager::loadFromFile(const std::string& filename) {
  // Reset state
  m_hasError = false;
  m_errorMessage.clear();
  m_serverConfigs.clear();
  
  // Validate filename
  if (!validateFilename(filename)) {
      m_hasError = true;
      m_errorMessage = "Invalid configuration filename: " + filename;
      return false;
  }
  
  // Read file content
  std::ifstream configFile(filename);
  if (!configFile.is_open()) {
      m_hasError = true;
      m_errorMessage = "Could not open configuration file: " + filename;
      return false;
  }
  
  std::stringstream buffer;
  buffer << configFile.rdbuf();
  std::string configContent = buffer.str();
  configFile.close();
  
  // Tokenize
  ConfigTokenizer tokenizer(configContent);
  auto tokens = tokenizer.tokenize();
  
  // Extract token values
  std::vector<std::string> tokenValues;
  for (const auto& token : tokens) {
      if (token.type != TokenType::EndOfFile) {
          tokenValues.push_back(token.value);
      }
  }
  
  // Validate token content
  if (!validateContent(tokenValues)) {
      m_hasError = true;
      m_errorMessage = "File does not appear to be a valid configuration file";
      return false;
  }
  
  // Parse
  ConfigParser parser(tokenValues);
  std::vector<ServerBlock> servers = parser.parse();
  
  if (parser.hasError()) {
      m_hasError = true;
      m_errorMessage = "Configuration file is incomplete or malformed";
      return false;
  }
  
  // Build runtime configuration
  m_serverConfigs = buildConfigs(servers);
  return true;
}

bool ConfigManager::validateFilename(const std::string& filename) {
  size_t dotPos = filename.find_last_of('.');
  if (dotPos != std::string::npos) {
      std::string extension = filename.substr(dotPos);
      return (extension == ".conf" || extension == ".config" || extension == ".cfg");
  }
  return false;
}

bool ConfigManager::validateContent(const std::vector<std::string>& tokens) {
  if (tokens.empty()) return false;
  
  // Check if first non-comment token is "server"
  for (const auto& token : tokens) {
      if (token == "server") return true;
      if (!token.empty() && token[0] != '#') return false;
  }
  
  return false;
}

std::vector<ServerConfig> ConfigManager::buildConfigs(const std::vector<ServerBlock>& blocks) {
  std::vector<ServerConfig> configs;
  for (const auto& block : blocks) {
      configs.push_back(buildServerConfig(block));
  }
  return configs;
}

HttpMethod stringToHttpMethod(const std::string& method) {
    if (method == "GET") return HttpMethod::GET;
    if (method == "POST") return HttpMethod::POST;
    if (method == "DELETE") return HttpMethod::DELETE;
    return HttpMethod::UNKNOWN; // Default to UNKNOWN for unsupported methods
}

ServerConfig ConfigManager::buildServerConfig(const ServerBlock& block) {
  ServerConfig config;

  for (const Directive& dir : block.directives) {
      if (dir.name == "listen" && !dir.args.empty())
          config.port = std::stoi(dir.args[0]);
      else if (dir.name == "server_name")
          config.server_names = dir.args;
      else if (dir.name == "error_page" && dir.args.size() >= 2 && dir.args[0] == "404")
          config.error_page_404 = dir.args[1];
      else if (dir.name == "client_max_body_size" && !dir.args.empty())
          config.client_max_body_size = std::stoul(dir.args[0]);
  }

  for (const LocationBlock& loc : block.locations) {
      RouteConfigFromConfigFile route;
      route.path = loc.path;

      for (const Directive& dir : loc.directives) {
        if (dir.name == "root" && !dir.args.empty())
            route.root = dir.args[0];
        else if (dir.name == "default_file" && !dir.args.empty())
            route.default_file = dir.args[0];
        else if (dir.name == "autoindex" && !dir.args.empty())
            route.autoindex = (dir.args[0] == "on");
        else if (dir.name == "methods")
            for (const std::string& method : dir.args) {
                route.allowed_methods.push_back(stringToHttpMethod(method));
            }
        else if (dir.name == "upload_directory" && !dir.args.empty())
            route.upload_dir = dir.args[0];
        else if (dir.name == "cgi" && dir.args.size() == 2)
            route.cgi_handlers[dir.args[0]] = dir.args[1];
      }

      config.routes.push_back(route);
  }

  return config;
}

void ConfigManager::printConfigs() const {
  for (const auto& config : m_serverConfigs) {
      std::cout << "==== Server Config ====\n";
      std::cout << "Port: " << config.port << "\n";
      std::cout << "Server Names: ";
      for (const auto& name : config.server_names)
          std::cout << name << " ";
      std::cout << "\nClient Max Body Size: " << config.client_max_body_size << "\n";
      std::cout << "Error Page 404: " << config.error_page_404 << "\n";

      for (const auto& route : config.routes) {
          std::cout << "  Location: " << route.path << "\n";
          std::cout << "    Root: " << route.root << "\n";
          std::cout << "    Default File: " << route.default_file << "\n";
          std::cout << "    Autoindex: " << (route.autoindex ? "on" : "off") << "\n";
          std::cout << "    Methods: ";
          for (const auto& m : route.allowed_methods)
              std::cout << m << " ";
          std::cout << "\n    Upload Dir: " << route.upload_dir << "\n";
          for (const auto& [ext, handler] : route.cgi_handlers)
              std::cout << "    CGI: " << ext << " => " << handler << "\n";
      }
  }
}

// ConfigTokenizer implementation
ConfigTokenizer::ConfigTokenizer(const std::string& input) : m_input(input), m_pos(0) {}

std::vector<Token> ConfigTokenizer::tokenize() {
  std::vector<Token> tokens;

  std::regex tokenRegex(
      R"((\s+)|(#.*$)|("([^"\\]|\\.)*")|([{};])|([^\s{};"#]+))", std::regex::ECMAScript);

  std::smatch match;

  while (m_pos < m_input.size()) {
      std::string remaining = m_input.substr(m_pos);
      if (std::regex_search(remaining, match, tokenRegex)) {
          std::string tokenStr = match.str();

          if (match[1].matched) {
              // Skip whitespace
          } else if (match[2].matched) {
              tokens.push_back({TokenType::Comment, tokenStr});
          } else if (match[3].matched) {
              tokens.push_back({TokenType::String, tokenStr});
          } else if (match[5].matched) {
              tokens.push_back({TokenType::Symbol, tokenStr});
          } else if (match[6].matched) {
              tokens.push_back({TokenType::Identifier, tokenStr});
          }

          m_pos += match.position() + match.length();
      } else {
          break;
      }
  }

  tokens.push_back({TokenType::EndOfFile, ""});
  return tokens;
}

// ConfigParser implementation
ConfigParser::ConfigParser(const std::vector<std::string>& tokens)
  : m_tokens(tokens), m_pos(0), m_hasError(false) {}

std::vector<ServerBlock> ConfigParser::parse() {
  std::vector<ServerBlock> servers;
  while (!end()) {
      if (peek() == "server") {
          servers.push_back(parseServer());
      } else {
          std::cerr << "Error: Unexpected token: " << peek() << std::endl;
          advance();
          m_hasError = true;
      }
  }
  return servers;
}

std::string ConfigParser::peek() const {
  return m_pos < m_tokens.size() ? m_tokens[m_pos] : "";
}

std::string ConfigParser::advance() {
  return m_pos < m_tokens.size() ? m_tokens[m_pos++] : "";
}

bool ConfigParser::match(const std::string& expected) {
  if (peek() == expected) {
      advance();
      return true;
  }
  return false;
}

bool ConfigParser::end() const {
  return m_pos >= m_tokens.size();
}

ServerBlock ConfigParser::parseServer() {
  ServerBlock server;
  match("server");
  if (!match("{")) {
      std::cerr << "Error: Expected '{' after 'server'" << std::endl;
      m_hasError = true;
      return server;
  }
  
  while (!match("}")) {
      if (end()) {
          std::cerr << "Error: Unexpected end of file, expected '}'" << std::endl;
          m_hasError = true;
          break;
      }
      
      if (peek() == "location") {
          server.locations.push_back(parseLocation());
      } else {
          server.directives.push_back(parseDirective());
      }
  }
  
  return server;
}

LocationBlock ConfigParser::parseLocation() {
  LocationBlock location;
  match("location");
  
  if (end()) {
      std::cerr << "Error: Unexpected end of file, expected location path" << std::endl;
      m_hasError = true;
      return location;
  }
  
  location.path = advance();
  
  if (!match("{")) {
      std::cerr << "Error: Expected '{' after location path" << std::endl;
      m_hasError = true;
      return location;
  }
  
  while (!match("}")) {
      if (end()) {
          std::cerr << "Error: Unexpected end of file, expected '}'" << std::endl;
          m_hasError = true;
          break;
      }
      
      location.directives.push_back(parseDirective());
  }
  
  return location;
}

Directive ConfigParser::parseDirective() {
  Directive directive;
  
  if (end()) {
      std::cerr << "Error: Unexpected end of file, expected directive name" << std::endl;
      m_hasError = true;
      return directive;
  }
  
  directive.name = advance();
  
  while (peek() != ";" && !end()) {
      directive.args.push_back(advance());
  }
  
  if (!match(";")) {
      std::cerr << "Error: Expected ';' after directive " << directive.name << std::endl;
      m_hasError = true;
  }
  
  return directive;
}
