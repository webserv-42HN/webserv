#include "Request.hpp"

void Request::parseRequest(const std::string& raw) {

    std::istringstream request_stream(raw);
    parseRequestLine(request_stream);
    parseHeaders(request_stream);
    parseBody(raw);
    parseContentType();
    printRequest();
}

void Request::parseRequestLine(std::istringstream& raw_req) {
    std::string line;
    
    if (std::getline(raw_req, line))
    {
        // Strip trailing CR(\r) if present (handle CRLF)
        if(!line.empty() && line.back() == '\r')
            line.pop_back();
        std::istringstream first_line(line);
        first_line >> req_line.method >> req_line.url >> req_line.http_version;
        req_line.url = urlDecode(req_line.url);
    }
}

void Request::parseHeaders(std::istringstream& raw_req) {
    std::string line;
    while (std::getline(raw_req, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            break; // End of headers

        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) continue;

        std::string name = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        headers.push_back(std::make_pair(name, value));
    }
}

void Request::parseBody(const std::string& raw) {
    std::string content_length_value;
    std::string transfer_encoding_value;
    std::ostringstream oss;
    std::string line;

    size_t header_end = raw.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        std::cerr << "Malformed HTTP request: no header-body split\n";
        return;
    }
    body = raw.substr(header_end + 4); // Move past \r\n\r\n
    for (const std::pair<std::string, std::string>& header : headers) {
        if (header.first == "Content-Length") {
            content_length_value = header.second;
        } else if (header.first == "Transfer-Encoding") {
            transfer_encoding_value = header.second;
        }
    }
    std::istringstream body_stream(body);
    if (transfer_encoding_value == "chunked") {
        while (std::getline(body_stream, line)) {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            oss << line << "\n";
        }
        body = encodeChunkedBody(oss.str());
    }
}

void Request::parseContentType() {
    for (const std::pair<std::string, std::string>& header : headers) {
        if (header.first == "Content-Type") {
            content_type = header.second;
            // // Trim leading spaces
            // content_type.erase(content_type.begin(), std::find_if(content_type.begin(), content_type.end(), [](unsigned char ch) {
            //     return !std::isspace(ch);
            // }));
            // // Trim trailing spaces
            // content_type.erase(std::find_if(content_type.rbegin(), content_type.rend(), [](unsigned char ch) {
            //     return !std::isspace(ch);
            // }).base(), content_type.end());
        }
        if (header.first == "Content-Length")
        {
            // std::cout << "TEST TEST TEST";
            content_len = std::stoi(header.second);
            // std::cout << "\"" << content_len << "\"" << std::endl;
        }
        
    }
}

// this function is used to make sure url is in the correct format
std::string Request::urlDecode(const std::string &src) {
    std::string ret;
    char ch;
    int i, ii;
    for (i = 0; i < (int)src.length(); i++) {
        if (src[i] == '%') {
            sscanf(src.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        } else if (src[i] == '+') {
            ret += ' ';
        } else {
            ret += src[i];
        }
    }
    return ret;
}