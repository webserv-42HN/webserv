#include "../includes/Response.hpp"

Response::Response() {
    // Register routes and corresponding handlers
    routes["/"] = std::bind(&Response::RootHandler, this, std::placeholders::_1);
    routes["/about"] = std::bind(&Response::AboutHandler, this, std::placeholders::_1);
    routes["/docs"] = std::bind(&Response::DocsHandler, this, std::placeholders::_1);
}

Response::~Response() {
    // Destructor
}

std::string Response::getResponse(std::string path, int statusCode) {
    std::ifstream file(path, std::ifstream::binary);
    if (!file.is_open())
        return "";
    file.seekg(0, std::ios::end);
    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string body(size, '\0');
    if (!file.read(&body[0], size)) {
        file.close();
        return ""; // Handle read error
    }
    file.close();
    std::ostringstream response;
    response    << getStatusLine(statusCode)
                << "Content-Type: text/html\r\n"
                << "Content-Length: " << body.size() << "\r\n"
                << "Connection: close\r\n"
                << "\r\n"
                << body;
    return response.str();
}

t_routeConfig Response::RootHandler(std::string url) {
    t_routeConfig config;
    (void)url;
    config.allowed_methods = {GET, POST};
    config.root_dir = "./www";
    config.redirect_to = "";
    config.autoindex = true;
    return config;
}

t_routeConfig Response::AboutHandler(std::string url) {
    t_routeConfig config;
    
    (void)url;
    config.allowed_methods = {GET};
    config.root_dir = "./www";
    config.redirect_to = "";
    config.autoindex = false;
    return config;
}

t_routeConfig Response::DocsHandler(std::string url) {
    t_routeConfig config;
    
    (void)url;
    config.allowed_methods = {GET};
    config.root_dir = "./www";
    config.redirect_to = "";
    config.autoindex = false;
    return config;
}


// t_routeConfig Response::ErrorHandler() {
//     t_routeConfig config;
    
//     config.allowed_methods = {GET};
//     config.root_dir = error_dir;
//     config.redirect_to = "";
//     config.autoindex = false;
//     return config;
// }

t_routeConfig Response::NotFoundHandler() {
    t_routeConfig config;
    
    config.allowed_methods = {GET};
    config.root_dir = error_dir;
    config.redirect_to = "";
    config.autoindex = false;
    return config;
}

std::string Response::getErrorResponse(int statusCode) {
    std::string error_page = error_dir + std::to_string(statusCode) + "_error.html";
    return getResponse(error_page, statusCode);
}

t_routeConfig Response::getRouteConfig(std::string url) {
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

std::string Response::routing(std::string method, std::string url) {
    HttpMethod http_method = methodToEnum(method);
    t_routeConfig config = getRouteConfig(url);
    std::string response;

    // Handle redirects first
    if (!config.redirect_to.empty()) {
        response = "HTTP/1.1 301 Moved Permanently\r\n";
        response += "Location: " + config.redirect_to + "\r\n\r\n";
        return response;
    }

    // Check if method is allowed
    auto it = std::find(config.allowed_methods.begin(),
                        config.allowed_methods.end(),
                        http_method);
    if (it == config.allowed_methods.end()) {
        return getErrorResponse(405);
    }

    // Build file path
    std::string full_path = config.root_dir + url;
    if (isDirectory(full_path)) {
        std::string index_path = full_path + "/index.html";
        std::ifstream index_file(index_path);
        if (index_file.good()) {
            return getResponse(index_path, 200);
        }
        if (config.autoindex) {
            return generateDirectoryListing(full_path);
        } else {
            return getErrorResponse(403);
        }
    } else {
        // Check if file exists
        std::ifstream file(full_path);
        if (!file.good()) {
            return getErrorResponse(404);
        }
        // Serve the file
        return getResponse(full_path, 200);
    }
}
