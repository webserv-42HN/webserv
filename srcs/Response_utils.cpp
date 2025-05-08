#include "../includes/Response.hpp"

bool Response::isDirectory(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false; // Error occurred
    return (info.st_mode & S_IFDIR) != 0; // Check if it's a directory
}

HttpMethod Response::methodToEnum(std::string method) {
    if (method == "GET") return GET;
    if (method == "POST") return POST;
    if (method == "DELETE") return DELETE;
    return UNKNOWN; // Invalid method
}

std::string Response::getStatusLine(int statusCode) {
    std::string reason;
    switch (statusCode) {
        case 200: reason = "OK"; break;
        case 400: reason = "Bad Request"; break;
        case 404: reason = "Not Found"; break;
        case 500: reason = "Internal Server Error"; break;
        default: reason = "Unknown"; break;
    }
    return "HTTP/1.1 " + std::to_string(statusCode) + " " + reason + "\r\n";    
}

std::string Response::generateDirectoryListing(const std::string& path) {
    std::string content = "<html><head><title>Index of " + path + "</title></head><body>";
    content += "<h1>Index of " + path + "</h1><hr><pre>";

    DIR* dir = opendir(path.c_str());
    if (dir != NULL) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            content += "<a href=\"" + std::string(entry->d_name) + "\">";
            content += std::string(entry->d_name) + "</a>\n";
        }
        closedir(dir);
    }
    content += "</pre><hr></body></html>";
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + std::to_string(content.length()) + "\r\n\r\n";
    response += content;
    return response;
}
