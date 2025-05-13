#include <regex>
#include <string>
#include <vector>
#include <iostream>

enum class TokenType { Identifier, Symbol, String, Comment, EndOfFile };

struct Token {
    TokenType type;
    std::string value;
};

class RegexTokenizer {
  public:
    RegexTokenizer(const std::string& input) : input(input), pos(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;

        std::regex tokenRegex(
            R"((\s+)|(#.*$)|("([^"\\]|\\.)*")|([{};])|([^\s{};"#]+))", std::regex::ECMAScript);

        std::smatch match;

        while (pos < input.size()) {
            std::string remaining = input.substr(pos);
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

                pos += match.position() + match.length();
            } else {
                break;
            }
        }

        tokens.push_back({TokenType::EndOfFile, ""});
        return tokens;
    }

  private:
    std::string input;
    size_t pos;
};



//parser
// Basic AST node definitions for the parser
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <iostream>

struct Directive {
    std::string name;
    std::vector<std::string> args;
};

struct LocationBlock {
    std::string path;
    std::vector<Directive> directives;
};

struct ServerBlock {
    std::vector<Directive> directives;
    std::vector<LocationBlock> locations;
};

class Parser {
public:
    Parser(const std::vector<std::string>& tokens)
        : tokens(tokens), pos(0) {}

    std::vector<ServerBlock> parse() {
        std::vector<ServerBlock> servers;
        while (!end()) {
            if (peek() == "server") {
                servers.push_back(parseServer());
            } else {
                std::cerr << "Unexpected token: " << peek() << std::endl;
                advance();
            }
        }
        return servers;
    }

private:
    const std::vector<std::string>& tokens;
    size_t pos;

    std::string peek() const {
        return pos < tokens.size() ? tokens[pos] : "";
    }

    std::string advance() {
        return pos < tokens.size() ? tokens[pos++] : "";
    }

    bool match(const std::string& expected) {
        if (peek() == expected) {
            advance();
            return true;
        }
        return false;
    }

    bool end() const {
        return pos >= tokens.size();
    }

    ServerBlock parseServer() {
        ServerBlock server;
        match("server");
        match("{");
        while (!match("}")) {
            if (peek() == "location") {
                server.locations.push_back(parseLocation());
            } else {
                server.directives.push_back(parseDirective());
            }
        }
        return server;
    }

    LocationBlock parseLocation() {
        LocationBlock location;
        match("location");
        location.path = advance();
        match("{");
        while (!match("}")) {
            location.directives.push_back(parseDirective());
        }
        return location;
    }

    Directive parseDirective() {
        Directive directive;
        directive.name = advance();
        while (peek() != ";" && !end()) {
            directive.args.push_back(advance());
        }
        match(";");
        return directive;
    }
};

void printDirective(const Directive& directive, int indent) {
    std::string indentStr(indent, ' ');
    std::cout << indentStr << directive.name;
    for (const auto& arg : directive.args) {
        std::cout << " " << arg;
    }
    std::cout << ";\n";
}

void printLocation(const LocationBlock& location, int indent) {
    std::string indentStr(indent, ' ');
    std::cout << indentStr << "location " << location.path << " {\n";
    for (const auto& directive : location.directives) {
        printDirective(directive, indent + 4);
    }
    std::cout << indentStr << "}\n";
}

void printServer(const ServerBlock& server, int indent = 0) {
    std::string indentStr(indent, ' ');
    std::cout << indentStr << "server {\n";
    for (const auto& directive : server.directives) {
        printDirective(directive, indent + 4);
    }
    for (const auto& location : server.locations) {
        printLocation(location, indent + 4);
    }
    std::cout << indentStr << "}\n";
}

// === Runtime Configuration Structures ===
struct RouteConfig {
    std::string path;
    std::string root;
    std::string default_file;
    bool autoindex = false;
    std::vector<std::string> allowed_methods;
    std::map<std::string, std::string> cgi_handlers;
    std::string upload_dir;
};

struct ServerConfig {
    int port = 80;
    std::vector<std::string> server_names;
    std::string error_page_404;
    size_t client_max_body_size = 1024 * 1024;
    std::vector<RouteConfig> routes;
};

ServerConfig buildServerConfig(const ServerBlock& block) {
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
        RouteConfig route;
        route.path = loc.path;

        for (const Directive& dir : loc.directives) {
            if (dir.name == "root" && !dir.args.empty())
                route.root = dir.args[0];
            else if (dir.name == "default_file" && !dir.args.empty())
                route.default_file = dir.args[0];
            else if (dir.name == "autoindex" && !dir.args.empty())
                route.autoindex = (dir.args[0] == "on");
            else if (dir.name == "methods")
                route.allowed_methods = dir.args;
            else if (dir.name == "upload_directory" && !dir.args.empty())
                route.upload_dir = dir.args[0];
            else if (dir.name == "cgi" && dir.args.size() == 2)
                route.cgi_handlers[dir.args[0]] = dir.args[1];
        }

        config.routes.push_back(route);
    }

    return config;
}

std::vector<ServerConfig> buildAllConfigs(const std::vector<ServerBlock>& blocks) {
    std::vector<ServerConfig> configs;
    for (const auto& block : blocks) {
        configs.push_back(buildServerConfig(block));
    }
    return configs;
}

#include <fstream>
#include <sstream>

int main(int argc, char *argv[]) {
  std::string config;
  std::string filename = "webserv.conf";  // Default filename
  
  // Allow custom config file via command line
  if (argc > 1) {
      filename = argv[1];
  }
  
  // Read the file
  std::ifstream configFile(filename);
  if (!configFile.is_open()) {
      std::cerr << "Error: Could not open config file: " << filename << std::endl;
      return 1;
  }
  
  // Read the entire file into the config string
  std::stringstream buffer;
  buffer << configFile.rdbuf();
  config = buffer.str();
  
  // Close the file
  configFile.close();


  RegexTokenizer tokenizer(config);
  auto tokens = tokenizer.tokenize();

  std::vector<std::string> tokenValues;
  for (const auto& token : tokens) {
      if (token.type != TokenType::EndOfFile) {
          tokenValues.push_back(token.value);
      }
  }

  Parser parser(tokenValues);
  std::vector<ServerBlock> servers = parser.parse();

  for (const auto& server : servers) {
      printServer(server);
  }

  std::vector<ServerConfig> serverConfigs = buildAllConfigs(servers);
  // Optional: print or use serverConfigs here
  for (const auto& config : serverConfigs) {
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

  return 0;
}
