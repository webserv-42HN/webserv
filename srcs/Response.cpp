#include "../includes/Response.hpp"
#include "../includes/Router.hpp"
#include "../includes/Request.hpp"

Response::Response() {
}

Response::~Response() {
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

std::string responseApplication(std::string body) {
    std::string resBody = "<html><body><h2>Submitted Form Data:</h2><ul>";
    std::istringstream iss(body);
    std::string pair;

    // Split by '&' first
    while (std::getline(iss, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            // URL decode the value
            // Simple URL decode for common characters
            size_t pos_plus;
            while ((pos_plus = value.find('+')) != std::string::npos) {
                value.replace(pos_plus, 1, " ");
            }
            resBody += "<li> <strong>" + key + ":</strong> " + value + "</li>";
        }
    }
    resBody += "</ul></body></html>";
    return resBody;
}

std::string Response::getPostResponse(std::string path) {
    std::string resBody;
    (void)path;
    // Check content type first
    std::cout << "TEST TEST TEST" << content_type << std::endl;
    if (content_type.empty()) {
        return getErrorResponse(400); // Bad Request - No content type
    }
    // Handle different content types
    // TODO remove space in
    if (content_type == " application/x-www-form-urlencoded") {
        if (body.empty()) {
            return getErrorResponse(400); // Bad Request - Empty body
        }
        resBody = responseApplication(body);
    } 
    else if (content_type.find("multipart/form-data") != std::string::npos) {
        // Handle file uploads
        // if (handleFileUpload(path)) {
            resBody = "<html><body><h1>File uploaded successfully</h1></body></html>";
        // } else {
            // return getErrorResponse(500); // Internal Server Error
        // }
    }
    else {
        return getErrorResponse(415); // Unsupported Media Type
    }
    return buildResponse(resBody, 200);
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
