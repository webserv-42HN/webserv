#include "../includes/Request.hpp"

// HTTP-message   = start-line CRLF
// *( field-line CRLF )
// CRLF
// [ message-body ]

// field-line   = field-name ":" OWS field-value OWS

// <METHOD> <URL> <HTTP_VERSION> ->Request line
// <Header-Field-1>: <value>
// <Header-Field-2>: <value>
// ...
// <Body (optional)>

// GET /index.html HTTP/1.1
// Host: www.example.com
// User-Agent: Mozilla/5.0
// Accept: text/html

void Request::parseRequest(std::string buf) {
    std::string line;
	std::istringstream raw_req(buf);
    
    parseRequestLine(raw_req);
    parseHeaders(raw_req);
    parseContentType();
    parseBody(raw_req);
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

void Request::parseContentType() {
    for (const auto& header : headers) {
        if (header.first == "Content-Type") {
            content_type = header.second;
            break;
        }
    }
}
