#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <regex>

typedef struct t_request
{
	std::string method;
	std::string url;
	std::string http_version;
}		s_request;

class Request
{
	protected:
		s_request req_line;
		std::vector<std::pair<std::string, std::string>> headers;
		std::string content_type;
		std::string body;
	public:
		bool isMalformedRequest(std::string &raw_req);
		std::string encodeChunkedBody(const std::string &body);
		void parseRequest(const std::string &buf);
		void parseRequestLine(std::istringstream& raw_req);
		void parseHeaders(std::istringstream& raw_req);
		void parseBody(std::istringstream& raw_req);
		void parseContentType();
		void printRequest();
		std::string urlDecode(const std::string &src);
		s_request getRequestLine();
};

#endif