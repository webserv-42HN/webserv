#include "../includes/Server.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"

void Request::printRequest() {
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