#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string.h>
#include <map>


typedef struct t_request
{
	std::string method;
	std::string url;
	std::string http_version;
}		s_request;

class Request
{
	private:
		s_request start_line;
		// std::map<>
	public:
		static void parseRequest(std::string buf);
};

// void Request::parseRequest(std::string buf)
// {
// 	std::istringstream stream(requestline);
// }

#endif