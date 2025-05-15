#pragma once
#include <string>

class Response {
	public:
		static std::string build(const std::string& body, const std::string& content_type = "text/html", int code = 200, const std::string& status = "OK");
};