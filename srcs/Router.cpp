#include "../includes/Router.hpp"

Router::Router() {
    // Register routes and corresponding handlers
    routes["/"] = std::bind(&Router::RootHandler, this, std::placeholders::_1);
    routes["/about"] = std::bind(&Router::AboutHandler, this, std::placeholders::_1);
    routes["/docs"] = std::bind(&Router::DocsHandler, this, std::placeholders::_1);
    routes["/uploads"] = std::bind(&Router::UploadsHandler, this, std::placeholders::_1);
    routes["/submit"] = std::bind(&Router::SubmitHandler, this, std::placeholders::_1);
}

Router::~Router() {
    // Destructor
}

t_routeConfig Router::getRouteConfig(std::string url) {
    // First try exact match
    auto it = routes.find(url);
    if (it != routes.end()) {
        return it->second(url);
    }
    // Try to find the closest matching route
    for (const auto& route : routes) {
        // Check if URL starts with the route path
        if (url.find(route.first) == 0) {
            return route.second(url);
        }
    }
    // If no route matches, return NotFoundHandler
    return NotFoundHandler();
}

t_routeConfig Router::RootHandler(std::string url) {
    t_routeConfig config;
    (void)url;
    config.allowed_methods = {GET, POST};
    config.root_dir = "./www";
    config.redirect_to = "";
    config.autoindex = false;
    return config;
}

t_routeConfig Router::AboutHandler(std::string url) {
    t_routeConfig config;
    
    (void)url;
    config.allowed_methods = {GET};
    config.root_dir = "./www";
    config.redirect_to = "";
    config.autoindex = false;
    return config;
}

t_routeConfig Router::UploadsHandler(std::string url) {
    t_routeConfig config;
    
    (void)url;
    config.allowed_methods = {GET, POST};
    config.root_dir = "./www";
    config.redirect_to = "";
    config.autoindex = false;
    return config;
}

t_routeConfig Router::DocsHandler(std::string url) {
    t_routeConfig config;
    
    (void)url;
    config.allowed_methods = {GET};
    config.root_dir = "./www";
    config.redirect_to = "";
    config.autoindex = false;
    return config;
}

t_routeConfig Router::SubmitHandler(std::string url) {
    t_routeConfig config;
    
    (void)url;
    config.allowed_methods = {POST};
    config.root_dir = "./www";
    config.redirect_to = "";
    config.autoindex = false;
    return config;
}

t_routeConfig Router::NotFoundHandler() {
    t_routeConfig config;
    
    config.allowed_methods = {GET};
    config.root_dir = error_dir;
    config.redirect_to = "";
    config.autoindex = false;
    return config;
}
