#pragma once
#include <string>

class Request {
	public:
		std::string method;
		std::string path;
		std::string version;

		static Request parse(const std::string & raw);
};