#include "../includes/Router.hpp"
#include "../includes/config_manager.hpp"


Router::Router(std::vector<ServerConfig> RouteConfigs) : routeConfigs(RouteConfigs) {
    for (const ServerConfig& server : routeConfigs) {
        for (const RouteConfigFromConfigFile& route : server.routes) {
            dynamicRoutes[route.path] = route;
        }
    }
}

Router::~Router() {
    // Destructor
}

t_routeConfig Router::getRouteConfig(std::string url) {
    // Exact match first
    auto it = dynamicRoutes.find(url);
    if (it != dynamicRoutes.end()) {
        return convertToRouteConfig(it->second);
    }
    // Partial match: find the longest matching prefix
    std::string bestMatch;
    for (const auto& route : dynamicRoutes) {
        if (url.find(route.first) == 0 && route.first.length() > bestMatch.length()) {
            bestMatch = route.first;
        }
    }
    if (!bestMatch.empty()) {
        return convertToRouteConfig(dynamicRoutes[bestMatch]);
    }
    return NotFoundHandler(); // fallback
}

t_routeConfig Router::convertToRouteConfig(const RouteConfigFromConfigFile& cfg) {
    t_routeConfig config;
    bool has_get = false;
    // Convert string methods to HttpMethod enums
    config.allowed_methods.clear();
    for (const auto& method : cfg.allowed_methods) {
        if (method == "GET") {
          config.allowed_methods.push_back(GET);
          has_get = true;
        }
        else if (method == "POST")
            config.allowed_methods.push_back(POST);
        else if (method == "DELETE")
            config.allowed_methods.push_back(DELETE);
        else if (method == "HEAD")
            config.allowed_methods.push_back(HEAD);
        // Add other methods as needed
    }
    if (has_get && std::find(config.allowed_methods.begin(), config.allowed_methods.end(), HEAD) == config.allowed_methods.end())
      config.allowed_methods.push_back(HEAD);
    
    // config.allowed_methods = cfg.allowed_methods;  //copilot fix
    config.root_dir = cfg.root;
    config.redirect_to = ""; // Set if needed
    config.autoindex = cfg.autoindex;
    config.client_max_body_size = cfg.client_max_body_size;
    std::cout << "DEBUG: DEFAULT FILE: " << cfg.default_file << std::endl;
    config.default_file = cfg.default_file;
    return config;
}

t_routeConfig Router::NotFoundHandler() {
    t_routeConfig config;

    config.allowed_methods = {GET};
    config.root_dir = "./www/error";
    config.redirect_to = "";
    config.autoindex = false;
    return config;
}
