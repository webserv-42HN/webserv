
// std::string Response::build(const std::string& body, const std::string& content_type, int code, const std::string& status) {
// 	std::ostringstream response;
// 	response << "HTTP/1.1 " << code << " " << status << "\r\n";
// 	response << "Content-Length: " << body.length() << "\r\n";
// 	response << "Content-Type: " << content_type << "\r\n";
// 	response << "connection: close\r\n\r\n";
// 	response << body;
// 	return response.str();
// }

#include "../includes/Response.hpp"
#include "../includes/Router.hpp"
#include "../includes/Request.hpp"
#include <limits>

Response::Response() {
}

Response::~Response() {
}

std::string Response::routing(std::string method, std::string url) {
    Router router;
    HttpMethod http_method = methodToEnum(method);
    t_responseRouteConfig config = router.getResponseRouteConfig(url);
    std::string response;
    std::string full_path;

    // Handle redirects first
    if (!config.redirect_to.empty()) {
        response = "HTTP/1.1 301 Moved Permanently\r\n";
        response += "Location: " + config.redirect_to + "\r\n\r\n";
        return response;
    }
    // Check if method is allowed
    std::cout << "[DEBUG] Method: " << method << std::endl;
    auto it = std::find(config.allowed_methods.begin(),
                        config.allowed_methods.end(),
                        http_method);
    if (it == config.allowed_methods.end())
    {
        std::cout << "[DEBUG] Method not allowed:" << method << std::endl;
        return getErrorResponse(405);
    }

    if (http_method == POST) {
        if (url == "/submit" || url == "/uploads")
            return getPostResponse(url);
        return getErrorResponse(404); // Not found for other POST paths
    }
    if (url.find("/uploads") == 0)
        full_path = "./www" + url;
    else
        full_path = config.root_dir + url;
    struct stat path_stat;
    if (stat(full_path.c_str(), &path_stat) != 0)
        return getErrorResponse(404);
    if (isDirectory(full_path)) {
        std::string index_path = full_path + "/index.html";
        std::ifstream index_file(index_path);
        if (index_file.good()) {
            index_file.close();
            return getGetResponse(index_path, 200);
        }
        if (config.autoindex) {
            return generateDirectoryListing(full_path);
        }
        return getErrorResponse(403);
    } 
    return generatingResponse(http_method, full_path);
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

std::string Response::getGetResponse(const std::string& requested_path, int statusCode) {
    std::ifstream file(requested_path, std::ifstream::binary);
    if (!file.is_open())
        return getErrorResponse(404);

    file.seekg(0, std::ios::end);
    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string body(size, '\0');
    if (!file.read(&body[0], size)) {
        file.close();
        return getErrorResponse(500); // Read error
    }

    file.close();
    return buildResponse(body, statusCode, getMimeType(requested_path));
}

std::string Response::getPostResponse(const std::string& url) {
    std::string resBody;
    std::string uploadedFile;

    if (content_type.empty())
        return getErrorResponse(400); // Bad Request - No Content-Type
    // Handle URL-encoded form submission (e.g., /submit)
    if (content_type == "application/x-www-form-urlencoded") {
        if (body.empty())
            return getErrorResponse(400);
        if (url == "/submit") {
            resBody = responseApplication(body);
            return buildResponse(resBody, 200, content_type);
        } else {
            return getErrorResponse(404); // Not found for this path
        }
    }
    // Handle file uploads (e.g., /uploads)
    if (content_type.find("multipart/form-data") != std::string::npos) {
        if (body.empty())
            return getErrorResponse(400);
        size_t boundary_pos = content_type.find("boundary=");
        if (boundary_pos == std::string::npos)
            return getErrorResponse(400);
        std::string boundary = "--" + content_type.substr(boundary_pos + 9);
        std::string upload_path = "./www/uploads/";
        struct stat st;
        if (stat(upload_path.c_str(), &st) == -1) {
            if (mkdir(upload_path.c_str(), 0755) == -1)
                return getErrorResponse(500);
        }
        bool success = handleFileUpload(upload_path, body, boundary, uploadedFile);
        std::cout << "[DEBUG] handleFileUpload result: " << (success ? "SUCCESS" : "FAILURE") << std::endl;
        if (success) {   
            // generateUploadsIndex(upload_path);
            return buildResponse(resBody, 200, content_type);
        } else {
            return getErrorResponse(500); // Upload failed
        }
    }
    return getErrorResponse(415); // Unsupported content-type
}

std::string Response::getDeleteResponse(const std::string& filepath) {
    struct stat st;
    std::cout << "[DEBUG] Deleting file: " << filepath << std::endl;
    if (stat(filepath.c_str(), &st) != 0) {
        return getErrorResponse(404); // Not found
    }
    if (remove(filepath.c_str()) != 0) {
        return getErrorResponse(500); // Failed to delete
    }
    return buildResponse("File deleted successfully", 200, "text/plain");
}

std::string Response::getErrorResponse(int statusCode) {
    std::string error_page = error_dir + std::to_string(statusCode) + "_error.html";
    return getGetResponse(error_page, statusCode);
}
