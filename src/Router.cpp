#include "../includes/Router.hpp"
#include "../includes/config_manager.hpp"


// Router::Router(std::vector<ServerConfig> RouteConfigs) : routeConfigs(RouteConfigs) {
//     // Register routes and corresponding handlers
//     routes["/"] = std::bind(&Router::RootHandler, this, std::placeholders::_1);
//     routes["/about"] = std::bind(&Router::AboutHandler, this, std::placeholders::_1);
//     routes["/docs"] = std::bind(&Router::DocsHandler, this, std::placeholders::_1);
//     routes["/uploads"] = std::bind(&Router::UploadsHandler, this, std::placeholders::_1);
//     routes["/submit"] = std::bind(&Router::SubmitHandler, this, std::placeholders::_1);
//     routes["/cgi"] = std::bind(&Router::cgiHandler, this, std::placeholders::_1);
// }

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

// t_routeConfig Router::getRouteConfig(std::string url) {
//     // First try exact match
//     auto it = routes.find(url);
//     if (it != routes.end()) {
//         return it->second(url);
//     }
//     // Try to find the closest matching route
//     for (const auto& route : routes) {
//         // Check if URL starts with the route path
//         if (url.find(route.first) == 0) {
//             return route.second(url);
//         }
//     }
//     // If no route matches, return NotFoundHandler
//     return NotFoundHandler();
// }

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

    // Convert string methods to HttpMethod enums
    config.allowed_methods.clear();
    for (const auto& method : cfg.allowed_methods) {
        if (method == "GET")
            config.allowed_methods.push_back(GET);
        else if (method == "POST")
            config.allowed_methods.push_back(POST);
        else if (method == "DELETE")
            config.allowed_methods.push_back(DELETE);
        else if (method == "HEAD")
            config.allowed_methods.push_back(HEAD);
        // Add other methods as needed
    }
    // config.allowed_methods = cfg.allowed_methods;  //copilot fix
    config.root_dir = cfg.root;
    config.redirect_to = ""; // Set if needed
    config.autoindex = cfg.autoindex;
    config.default_file = cfg.default_file;
    return config;
}

// t_routeConfig Router::RootHandler(std::string url) {
//     t_routeConfig config;
//     (void)url;
//     config.allowed_methods = {GET, POST, DELETE}; // Assuming the first server config
//     config.root_dir = "./www";
//     config.redirect_to = "";
//     config.autoindex = false;
//     return config;
// }

// t_routeConfig Router::AboutHandler(std::string url) {
//     t_routeConfig config;

//     (void)url;
//     config.allowed_methods = {GET};
//     config.root_dir = "./www";
//     config.redirect_to = "";
//     config.autoindex = true;
//     return config;
// }

// t_routeConfig Router::UploadsHandler(std::string url) {
//     t_routeConfig config;

//     (void)url;
//     config.allowed_methods = {GET, POST, DELETE};
//     config.root_dir = "./www";
//     config.redirect_to = "";
//     config.autoindex = true;
//     return config;
// }

// t_routeConfig Router::DocsHandler(std::string url) {
//     t_routeConfig config;

//     (void)url;
//     config.allowed_methods = {GET};
//     config.root_dir = "./www";
//     config.redirect_to = "";
//     config.autoindex = false;
//     return config;
// }

// t_routeConfig Router::SubmitHandler(std::string url) {
//     t_routeConfig config;

//     (void)url;
//     config.allowed_methods = {GET, POST, DELETE};
//     config.root_dir = "./www";
//     config.redirect_to = "";
//     config.autoindex = false;
//     return config;
// }

// t_routeConfig Router::cgiHandler(std::string url) {
//     t_routeConfig config;

//     (void)url;
//     config.allowed_methods = {GET, POST};
//     config.root_dir = "./www";
//     config.redirect_to = "";
//     config.autoindex = true;
//     return config;
// }

t_routeConfig Router::NotFoundHandler() {
    t_routeConfig config;

    config.allowed_methods = {GET};
    config.root_dir = "./www/error";
    config.redirect_to = "";
    config.autoindex = false;
    return config;
}
