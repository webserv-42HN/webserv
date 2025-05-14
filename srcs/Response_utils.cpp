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

bool Response::handleFileUpload(const std::string& path, const std::string& body, const std::string& boundary) {
    size_t pos = 0;
    size_t boundary_len = boundary.length();

    while ((pos = body.find(boundary, pos)) != std::string::npos) {
        pos += boundary_len;

        // Find the headers section
        size_t headers_end = body.find("\r\n\r\n", pos);
        if (headers_end == std::string::npos) {
            return false; // Malformed request
        }

        // Extract headers
        std::string headers = body.substr(pos, headers_end - pos);
        pos = headers_end + 4; // Move past the headers and the empty line

        // Parse Content-Disposition to get the filename
        size_t filename_pos = headers.find("filename=\"");
        if (filename_pos == std::string::npos) {
            continue; // No file in this part, skip
        }
        filename_pos += 10; // Move past "filename=\""
        size_t filename_end = headers.find("\"", filename_pos);
        if (filename_end == std::string::npos) {
            return false; // Malformed filename
        }
        std::string filename = headers.substr(filename_pos, filename_end - filename_pos);

        // Extract the file content
        size_t file_end = body.find(boundary, pos);
        if (file_end == std::string::npos) {
            return false; // Malformed request
        }
        std::string file_content = body.substr(pos, file_end - pos - 2); // Exclude trailing \r\n
        pos = file_end;

        // Save the file to the server
        std::ofstream outfile(path + "/" + filename, std::ios::binary);
        if (!outfile.is_open()) {
            return false; // Failed to open file for writing
        }
        outfile.write(file_content.c_str(), file_content.size());
        outfile.close();
    }

    return true;
}
