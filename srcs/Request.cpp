#include "../includes/Request.hpp"

// HTTP-message   = start-line CRLF
// *( field-line CRLF )
// CRLF
// [ message-body ]

// <METHOD> <URL> <HTTP_VERSION> ->Request line
// <Header-Field-1>: <value>
// <Header-Field-2>: <value>
// ...
// <Body (optional)>
// field-line   = field-name ":" OWS field-value OWS

// GET /index.html HTTP/1.1
// Host: www.example.com
// User-Agent: Mozilla/5.0
// Accept: text/html

void Request::parseRequest(std::string buf) {
    std::string line;
	std::istringstream raw_req(buf);
    
    parseRequestLine(raw_req);
    parseHeaders(raw_req);
    parseBody(raw_req);
    std::cout << "---request line---" << std::endl;
    std::cout << "method: " << req_line.method;
    std::cout << " | url: " << req_line.url;
    std::cout << " | http_version: " << req_line.http_version << std::endl;
    std::cout << "---headers---" << std::endl;
    for (const auto& headers : headers)
        std::cout << headers.first << " : " << headers.second << std::endl;
    std::cout << "---body---" << std::endl;
    std::cout << body << std::endl;
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
    }
}

void Request::parseHeaders(std::istringstream& raw_req) {
    std::string line;

    while (std::getline(raw_req, line)) {
        if (line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            break;  // End of headers
        // Strip CR if line ends in CRLF
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        // Check colon position
        auto colon_pos = line.find(':');
        if (colon_pos == std::string::npos || colon_pos == 0) 
            return;
        std::string header_name = line.substr(0, colon_pos);
        std::string header_value = line.substr(colon_pos + 1);
        headers.push_back(std::make_pair(header_name, header_value));
    }
}

void Request::parseBody(std::istringstream& raw_req) {
    // Check if there is a body
    if(raw_req.peek() != EOF) {
        std::getline(raw_req, body);
        if(!body.empty() && body.back() == '\r')
            body.pop_back();
        for(auto& val : headers) {
            if(val.first == "Transfer-Encoding" && val.second == "chunked") {
                body = encodeChunkedBody(body);
                break;
            }
        }
    }
}

bool Request::isMalformedRequest(std::string& raw_req) {
    std::istringstream iss(raw_req);
    std::string line;
    std::string method, path, version;

    // === 1. Check the request line ===
    if (!std::getline(iss, line)) return true;
    // Strip CR if line ends in CRLF
    if (!line.empty() && line.back() == '\r') 
        line.pop_back();
    std::istringstream request_line(line);
    request_line >> method >> path >> version;
    if (method.empty() || path.empty() || version.empty()) return true;   // Validate method is uppercase and alphabetic
    if (!std::regex_match(method, std::regex("^[A-Z]+$"))) return true;
    if (path[0] != '/') return true; // Validate path starts with '/'
    if (!std::regex_match(version, std::regex("^HTTP/\\d\\.\\d$"))) return true; // Validate HTTP version format
    // === 2. Check headers ===
    bool header_section_started = false;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break;  // End of headers
        header_section_started = true;
        // Reject headers starting with whitespace (potential smuggling)
        if (line[0] == ' ' || line[0] == '\t') return true;
        // Check colon position
        auto colon_pos = line.find(':');
        if (colon_pos == std::string::npos || colon_pos == 0) return true;
        // Optional: Check for control characters in header name
        for (size_t i = 0; i < colon_pos; ++i) {
            if (iscntrl(line[i])) return true;
        }
    }
    if (!header_section_started) return true; // If no blank line after headers, malformed
    return false;  // Request appears valid
}

// "4\r\nWiki\r\n"
// "5\r\npedia\r\n"
// "E\r\n in\r\n\r\nchunks.\r\n"
// "0\r\n\r\n"

std::string Request::encodeChunkedBody(std::string& body) {
    std::istringstream iss(body);
    std::string decoded_body;
    std::string line;
    int chunk_size = 0;
    std::istringstream sizeStream;
    char *buf;

    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        sizeStream.str(line);
        sizeStream >> std::hex >> chunk_size;
        if (chunk_size == 0)
            break; // End of chunks
        buf = new char[chunk_size];
        iss.read(buf, chunk_size);
        decoded_body.append(buf, chunk_size);
        delete[] buf;
        iss.ignore(2); // Ignore CRLF after chunk
    }
    return decoded_body;
}
