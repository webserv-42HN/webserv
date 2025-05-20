#include "Response.hpp"
#include <sstream>

std::string Response::build(const std::string& body, const std::string& content_type, int code, const std::string& status) {
	std::ostringstream response;
	response << "HTTP/1.1 " << code << " " << status << "\r\n";
	response << "Content-Length: " << body.length() << "\r\n";
	response << "Content-Type: " << content_type << "\r\n";
	response << "connection: close\r\n\r\n";
	response << body;
	return response.str();
}