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
    if (method == "HEAD") return HEAD;
    return UNKNOWN; // Invalid method
}

std::string Response::getStatusLine(int statusCode) {
    std::string reason;
    switch (statusCode) {
        case 200: reason = "OK"; break;
        case 400: reason = "Bad Request"; break;
        case 404: reason = "Not Found"; break;
        case 500: reason = "Internal Server Error"; break;
        case 405: reason = "Method Not Allowed"; break;
        default: reason = "Unknown"; break;
    }
    return "HTTP/1.1 " + std::to_string(statusCode) + " " + reason + "\r\n";
}

// std::string Response::generateDirectoryListing(const std::string& path) {
//     std::string content = "<html><head><title>Index of " + path + "</title></head><body>";
//     content += "<h1>Index of " + path + "</h1><hr><ul class=\"file-list\">";

//     DIR* dir = opendir(path.c_str());
//     if (dir != NULL) {
//         struct dirent* entry;
//         while ((entry = readdir(dir)) != NULL) {
//             if (entry->d_name[0] != '.') {  // Skip hidden files
//                 std::string filename = entry->d_name;
//                 content += "<li><a href=\"/" + filename + "\">";
//                 content += filename + "</a></li>";
//             }
//         }
//         closedir(dir);
//     }

//     content += "</ul><hr></body></html>";
//     return buildResponse(content, 200, content_type);
// }

std::string Response::generateDirectoryListing(const std::string& fsPath, const std::string& urlPath) {
    std::string content = "<html><head><title>Index of " + urlPath + "</title></head><body>";
    content += "<h1>Index of " + urlPath + "</h1><hr><ul class=\"file-list\">";

    DIR* dir = opendir(fsPath.c_str());
    if (dir != NULL) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') {  // Skip hidden files
                std::string filename = entry->d_name;

                std::string fileUrl = urlPath;
                if (fileUrl.back() != '/') {
                    fileUrl += '/';
                }
                fileUrl += filename;

                content += "<li><a href=\"" + fileUrl + "\">" + filename + "</a></li>";
            }
        }
        closedir(dir);
    }

    content += "</ul><hr></body></html>";
    return buildResponse(content, 200, content_type);
}

size_t Response::getContentLength(const std::string &headers) const {
    size_t contentLength = 0;
    size_t pos = headers.find("Content-Length:");
    if (pos != std::string::npos) {
        size_t start = headers.find_first_of("0123456789", pos);
        size_t end = headers.find("\r\n", start);
        contentLength = std::atoi(headers.substr(start, end - start).c_str());
        return contentLength;
    }
    return 0;
}

std::string Response::buildResponse(const std::string& body, int statusCode, const std::string& contentType) {
    std::stringstream res;
    if (statusCode == 404)
        res << "HTTP/1.1 " << statusCode << " Not Found\r\n";
    if (statusCode == 400)
        res << "HTTP/1.1 " << statusCode << " Bad Request\r\n";
    if (statusCode == 405)
        res << "HTTP/1.1 " << statusCode << " Method Not Allowed\r\n";
    if (statusCode == 500)
        res << "HTTP/1.1 " << statusCode << " Internal Server Error\r\n";
    if (statusCode == 403)
        res << "HTTP/1.1 " << statusCode << " Forbidden\r\n";
    if (statusCode == 413)
        res << "HTTP/1.1 " << statusCode << " Payload Too Large\r\n";
    if (statusCode == 200)
        res << "HTTP/1.1 " << statusCode << " OK\r\n";
    res << "Content-Type: " << contentType << "\r\n";
    res << "Content-Length: " << body.size() << "\r\n";
    res << "\r\n";
    res << body;
    return res.str();
}


std::string Response::responseTextPlain(const std::string& body) {
    std::string response;
    response += "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    response += "\r\n";
    response += body;
    return response;
}