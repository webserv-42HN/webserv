#include "../includes/Response.hpp"

bool Response::handleFileUpload(const std::string& path, const std::string& body, const std::string& boundary, std::string& out_filename) {
    size_t pos = 0;
    size_t boundary_len = boundary.length();
    bool fileSaved = false;

    std::cout << "TEST TEST TEST" << std::endl;
    while ((pos = body.find(boundary, pos)) != std::string::npos)
    {
        pos += boundary_len;
        if (body.substr(pos, 2) == "\r\n")
            pos += 2;
        size_t headers_end = body.find("\r\n\r\n", pos);
        if (headers_end == std::string::npos) break;
        std::string headers = body.substr(pos, headers_end - pos);
        pos = headers_end + 4;
        size_t filename_pos = headers.find("filename=\"");
        if (filename_pos == std::string::npos) continue;
        filename_pos += 10;
        size_t filename_end = headers.find("\"", filename_pos);
        if (filename_end == std::string::npos) break;
        std::string filename = headers.substr(filename_pos, filename_end - filename_pos);
        if (filename.empty()) continue;
        out_filename = filename;
        size_t file_end = body.find(boundary, pos);
        if (file_end == std::string::npos) break;
        std::string file_content = body.substr(pos, file_end - pos);
        // Strip possible trailing \r\n
        if (file_content.size() >= 2 && file_content.substr(file_content.size() - 2) == "\r\n")
            file_content = file_content.substr(0, file_content.size() - 2);
        std::string full_path = path + "/" + filename;
        std::cout << "Saving file: " << full_path << std::endl;
        std::ofstream outfile(full_path, std::ios::binary);
        if (!outfile) {
            std::cerr << "[ERROR] Cannot write to: " << full_path << std::endl;
            return false;
        }
        outfile.write(file_content.data(), file_content.size());
        outfile.close();
        fileSaved = true;
        break; // Only handle one file per request
    }
    return fileSaved;
}

static bool endsWith(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length()) return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string Response::getMimeType(const std::string& path) {
    if (endsWith(path, ".html")) return "text/html";
    if (endsWith(path, ".txt")) return "text/plain";
    if (endsWith(path, ".jpg") || endsWith(path, ".jpeg")) return "image/jpeg";
    if (endsWith(path, ".png")) return "image/png";
    if (endsWith(path, ".css")) return "text/css";
    if (endsWith(path, ".js")) return "application/javascript";
    if (endsWith(path, ".pdf")) return "application/pdf";
    if (endsWith(path, ".docx")) return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    if (endsWith(path, ".doc")) return "application/msword";
    // Add more as needed
    return "application/octet-stream";
}

std::string Response::responseApplication(std::string body) {
    std::string resBody = "<html><body><h2>Submitted Form Data:</h2><ul>";
    std::istringstream iss(body);
    std::string pair;

    std::map<std::string, std::string> data;
    // Parse key-value pairs
    while (std::getline(iss, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            // Replace '+' with space
            size_t pos_plus;
            while ((pos_plus = value.find('+')) != std::string::npos) {
                value.replace(pos_plus, 1, " ");
            }
            data[key] = value;
            resBody += "<li><strong>" + key + ":</strong> " + value + "</li>";
        }
    }
    // // Ensure submit folder exists
    // std::string folder = ".www/submit";
    // struct stat st;
    // if (stat(folder.c_str(), &st) == -1) {
    //     mkdir(folder.c_str(), 0755);
    // }
    // // Choose filename (use name or fallback to timestamp)
    // std::string filename = "entry.txt";
    // if (data.find("name") != data.end() && !data["name"].empty()) {
    //     filename = data["name"] + ".txt";
    // } else {
    //     std::time_t now = std::time(nullptr);
    //     filename = "entry_" + std::to_string(now) + ".txt";
    // }
    // std::ofstream file(folder + "/" + filename);
    // if (file.is_open()) {
    //     for (const auto& [key, value] : data) {
    //         file << key << ": " << value << "\n";
    //     }
    //     file.close();
    // }
    resBody += "</ul></body></html>";
    return resBody;
}
