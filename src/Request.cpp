#include "Request.hpp"
#include <sstream>

Request Request::parse(const std::string& raw) {
	Request req;
	std::istringstream stream(raw);
	stream >> req.method >> req.path >> req.version;
	return req;
}