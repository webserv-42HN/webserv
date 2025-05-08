#include "../includes/Response.hpp"
#include "../includes/Router.hpp"

Response::Response() {
    // Constructor
}

Response::~Response() {
    // Destructor
}

std::string Response::routing(std::string method, std::string url) {
    Router router;
    HttpMethod http_method = methodToEnum(method);
    t_routeConfig config = router.getRouteConfig(url);
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
    if (it == config.allowed_methods.end())
        return getErrorResponse(405);
    // Build file path
    std::string full_path = config.root_dir + url;
    if (isDirectory(full_path)) {
        std::string index_path = full_path + "/index.html";
        std::ifstream index_file(index_path);
        if (index_file.good())
            return getGetResponse(index_path, 200);
        if (config.autoindex)
            return generateDirectoryListing(full_path);
        else
            return getErrorResponse(403);
    } 
    else {
        return generatingResponse(http_method, full_path);
    }
}

std::string Response::generatingResponse(HttpMethod method, std::string full_url) {
    std::string response;
    switch (method)
    {
    case GET:
        response = getGetResponse(full_url, 200);
        break;
    case POST:
        response = getPostResponse(full_url);
        break;
    case DELETE:
        response = getDeleteResponse(full_url);
        break;
    case UNKNOWN:
        response = getErrorResponse(405);
        break;
    default:
        break;
    }
    return response;
}

std::string Response::buildResponse(std::string body, int statusCode) {
    std::ostringstream response;
    response    << getStatusLine(statusCode)
                << "Content-Type: text/html\r\n"
                << "Content-Length: " << body.size() << "\r\n"
                << "Connection: close\r\n"
                << "\r\n"
                << body;
    return response.str();
}

std::string Response::getGetResponse(std::string path, int statusCode) {
    std::ifstream file(path, std::ifstream::binary);
    if (!file.good())
        return getErrorResponse(404);
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
    return buildResponse(body, statusCode);
}

std::string Response::getPostResponse(std::string path) {
    // Handle POST request
    (void)path; // Suppress unused variable warning
    // For now, just return a simple response
    std::string body = "<html><body><h1>POST request received</h1></body></html>";
    return buildResponse(body, 200);
    // return getErrorResponse(501); // Not Implemented
}

std::string Response::getDeleteResponse(std::string path) {
    // Handle DELETE request
    (void)path; // Suppress unused variable warning
    // For now, just return a simple response
    std::string body = "<html><body><h1>DELETE request received</h1></body></html>";
    return buildResponse(body, 200);
    // return getErrorResponse(501); // Not Implemented
}

std::string Response::getErrorResponse(int statusCode) {
    std::string error_page = error_dir + std::to_string(statusCode) + "_error.html";
    return getGetResponse(error_page, statusCode);
}
